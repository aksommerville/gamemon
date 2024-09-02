#include "gamemon_internal.h"

/* Current time.
 */
 
double gamemon_time_now() {
  struct timeval tv={0};
  gettimeofday(&tv,0);
  return (double)tv.tv_sec+(double)tv.tv_usec/1000000.0;
}

/* Attempt connection.
 */
 
static int gamemon_connect(struct gamemon *gamemon) {
  
  /* When (baud_rate) set, we need to call stty first.
   * So in that case, we'll stat for the file's existence before moving on.
   * If (baud_rate) zero, just try opening it.
   */
  if (gamemon->baud_rate) {
    struct stat st;
    if (stat(gamemon->path,&st)<0) return 0;
    char cmd[1024];
    int cmdc=snprintf(cmd,sizeof(cmd),
      "stty -F %s cs8 %d ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts",
      gamemon->path,gamemon->baud_rate
    );
    if ((cmdc<1)||(cmdc>=sizeof(cmd))) return -1;
    if (system(cmd)) return -1;
  }
  
  /* Try to open.
   * If it fails, no big deal, just get out.
   */
  if ((gamemon->fd=open(gamemon->path,O_RDWR))<0) return 0;
  
  /* File is newly opened.
   * Send the framebuffer format request, and notify the delegate.
   */
  uint8_t request=0x01;
  if (write(gamemon->fd,&request,1)!=1) {
    // well that was nice while it lasted.
    close(gamemon->fd);
    gamemon->fd=-1;
    return 0;
  }
  if (gamemon->delegate.connected) gamemon->delegate.connected(gamemon->delegate.userdata);
  return 1;
}

/* Update.
 */

int gamemon_update(struct gamemon *gamemon) {
  if (!gamemon) return 0;
  
  /* If we're not connected, check the timer and maybe retry connecting.
   */
  if (gamemon->fd<0) {
    double now=gamemon_time_now();
    double elapsed=now-gamemon->retry_time;
    if (elapsed<GAMEMON_RETRY_INTERVAL) return 0;
    gamemon->retry_time=now;
    gamemon_connect(gamemon);
    if (gamemon->fd<0) return 0;
  }
  
  /* Poll for messages from device.
   */
  struct pollfd pollfd={.fd=gamemon->fd,.events=POLLIN|POLLERR|POLLHUP};
  if (poll(&pollfd,1,0)<=0) return 0;
  uint8_t buf[256];
  int bufc=read(gamemon->fd,buf,sizeof(buf));
  if (bufc<=0) {
    close(gamemon->fd);
    gamemon->fd=-1;
    gamemon->fbw=gamemon->fbh=gamemon->pixfmt=0;
    gamemon->retry_time=gamemon_time_now();
    if (gamemon->delegate.disconnected) gamemon->delegate.disconnected(gamemon->delegate.userdata);
    return 0;
  }
  int bufp=0;
  while (bufp<bufc) {
    uint8_t opcode=buf[bufp++];
    switch (opcode) {

      case 0x00: { // Abort.
          close(gamemon->fd);
          gamemon->fd=-1;
          gamemon->fbw=gamemon->fbh=gamemon->pixfmt=0;
          gamemon->retry_time=gamemon_time_now();
          if (gamemon->delegate.disconnected) gamemon->delegate.disconnected(gamemon->delegate.userdata);
          return 0;
        }
        
      case 0x03: { // Declare Format.
          if (bufp>bufc-5) return 0;
          int w=buf[bufp++]<<8;
          w|=buf[bufp++];
          int h=buf[bufp++]<<8;
          h|=buf[bufp++];
          int pixfmt=buf[bufp++];
          if (
            (w<1)||(w>GAMEMON_FRAMEBUFFER_SIZE_LIMIT)||
            (h<1)||(h>GAMEMON_FRAMEBUFFER_SIZE_LIMIT)||
            !gamemon_pixel_size(pixfmt)
          ) {
            fprintf(stderr,"gamemon: Invalid framebuffer format %dx%d@0x%02x\n",w,h,pixfmt);
            close(gamemon->fd);
            gamemon->fd=-1;
            gamemon->fbw=gamemon->fbh=gamemon->pixfmt=0;
            gamemon->retry_time=gamemon_time_now();
            if (gamemon->delegate.disconnected) gamemon->delegate.disconnected(gamemon->delegate.userdata);
            return 0;
          }
          if ((w!=gamemon->fbw)||(h!=gamemon->fbh)||(pixfmt!=gamemon->pixfmt)) {
            gamemon->fbw=w;
            gamemon->fbh=h;
            gamemon->pixfmt=pixfmt;
            if (gamemon->delegate.fb_format) gamemon->delegate.fb_format(w,h,pixfmt,gamemon->delegate.userdata);
          }
        } break;
        
      case 0x04: { // Input State.
          int state=buf[bufp++];
          if (state!=gamemon->input) {
            int pv=gamemon->input;
            gamemon->input=state;
            if (gamemon->delegate.input) gamemon->delegate.input(state,pv,gamemon->delegate.userdata);
          }
        } break;
        
      default: {
          fprintf(stderr,"gamemon: Unexpected opcode 0x%02x from device.\n",opcode);
          close(gamemon->fd);
          gamemon->fd=-1;
          gamemon->fbw=gamemon->fbh=gamemon->pixfmt=0;
          gamemon->retry_time=gamemon_time_now();
          if (gamemon->delegate.disconnected) gamemon->delegate.disconnected(gamemon->delegate.userdata);
          return 0;
        }
    }
  }
  return 0;
}

/* Send framebuffer.
 */
 
int gamemon_send_framebuffer(
  struct gamemon *gamemon,
  int w,int h,int pixfmt,
  const void *v,int c
) {
  if (!gamemon) return 0;
  if (gamemon->fd<0) return 0;
  if (!gamemon->fbw) return 0;
  if (!v) return -1;
  
  if ((w==gamemon->fbw)&&(h==gamemon->fbh)&&(pixfmt==gamemon->pixfmt)) {
    // Send it verbatim from the provided buffer. Just validate size.
    if (c!=gamemon_measure_framebuffer(w,h,pixfmt)) return -1;
    
  } else {
    // Convert into our temporary buffer and send from that.
    int reqlen=gamemon_measure_framebuffer(gamemon->fbw,gamemon->fbh,gamemon->pixfmt);
    if (reqlen<1) return -1;
    if (reqlen>gamemon->tmpa) {
      void *nv=malloc(reqlen);
      if (!nv) return -1;
      if (gamemon->tmp) free(gamemon->tmp);
      gamemon->tmp=nv;
      gamemon->tmpa=reqlen;
    }
    int tmpc=gamemon_fb_convert(
      gamemon->tmp,gamemon->tmpa,gamemon->fbw,gamemon->fbh,gamemon->pixfmt,
      v,c,w,h,pixfmt
    );
    if (tmpc<1) return -1;
    v=gamemon->tmp;
    c=tmpc;
  }
  
  // Send opcode then buffer.
  uint8_t opcode=0x02;
  if (
    (write(gamemon->fd,&opcode,1)!=1)||
    (write(gamemon->fd,v,c)!=c)
  ) {
    fprintf(stderr,"gamemon: write error\n");
    close(gamemon->fd);
    gamemon->fd=-1;
    gamemon->fbw=gamemon->fbh=gamemon->pixfmt=0;
    gamemon->retry_time=gamemon_time_now();
    if (gamemon->delegate.disconnected) gamemon->delegate.disconnected(gamemon->delegate.userdata);
    return 0;
  }
  return 1;
}
