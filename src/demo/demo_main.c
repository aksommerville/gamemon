#include "demo_internal.h"

struct demo demo={0};

/* Signal handler.
 */
 
static void demo_rcvsig(int sigid) {
  switch (sigid) {
    case SIGINT: if (++(demo.sigc)>=3) {
        fprintf(stderr,"demo: Too many unprocessed signals.\n");
        exit(1);
      } break;
  }
}

/* Draw 3x5 glyphs to y8 framebuffer.
 */
 
static const uint16_t demo_glyphv_3x5[95]={
#define GLYPH(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) (a<<15)|(b<<14)|(c<<13)|(d<<12)|(e<<11)|(f<<10)|(g<<9)|(h<<8)|(i<<7)|(j<<6)|(k<<5)|(l<<4)|(m<<3)|(n<<2)|(o<<1),
#define _ 0
#define X 1

  GLYPH( _,X,_,   _,X,_,   _,X,_,   _,_,_,   _,X,_ ) // !
  GLYPH( X,_,X,   X,_,X,   _,_,_,   _,_,_,   _,_,_ ) // "
  GLYPH( _,_,_,   X,X,X,   X,_,X,   X,X,X,   _,_,_ ) // #
  GLYPH( X,X,X,   X,X,_,   X,X,X,   _,X,X,   X,X,X ) // $
  GLYPH( X,_,X,   _,_,X,   _,X,_,   X,_,_,   X,_,X ) // %
  GLYPH( _,X,_,   X,_,_,   X,X,_,   X,_,X,   _,X,X ) // &
  GLYPH( _,X,_,   _,X,_,   _,_,_,   _,_,_,   _,_,_ ) // '
  GLYPH( _,_,X,   _,X,_,   _,X,_,   _,X,_,   _,_,X ) // (
  GLYPH( X,_,_,   _,X,_,   _,X,_,   _,X,_,   X,_,_ ) // )
  GLYPH( X,_,X,   _,X,_,   X,X,X,   _,X,_,   X,_,X ) // *
  GLYPH( _,_,_,   _,X,_,   X,X,X,   _,X,_,   _,_,_ ) // +
  GLYPH( _,_,_,   _,_,_,   _,_,_,   X,X,_,   _,X,_ ) // ,
  GLYPH( _,_,_,   _,_,_,   X,X,X,   _,_,_,   _,_,_ ) // -
  GLYPH( _,_,_,   _,_,_,   _,_,_,   _,_,_,   _,X,_ ) // .
  GLYPH( _,_,X,   _,_,X,   _,X,_,   X,_,_,   X,_,_ ) // /
  
  GLYPH( _,X,_,   X,_,X,   X,_,X,   X,_,X,   _,X,_ ) // 0
  GLYPH( X,X,_,   _,X,_,   _,X,_,   _,X,_,   X,X,X ) // 1
  GLYPH( X,X,_,   _,_,X,   _,X,_,   X,_,_,   X,X,X ) // 2
  GLYPH( X,X,_,   _,_,X,   _,X,_,   _,_,X,   X,X,_ ) // 3
  GLYPH( X,_,X,   X,_,X,   X,X,X,   _,_,X,   _,_,X ) // 4
  GLYPH( X,X,X,   X,_,_,   X,X,_,   _,_,X,   X,X,_ ) // 5
  GLYPH( _,X,_,   X,_,_,   X,X,_,   X,_,X,   _,X,_ ) // 6
  GLYPH( X,X,X,   _,_,X,   _,_,X,   _,_,X,   _,_,X ) // 7
  GLYPH( _,X,_,   X,_,X,   _,X,_,   X,_,X,   _,X,_ ) // 8
  GLYPH( _,X,X,   X,_,X,   _,X,X,   _,_,X,   _,_,X ) // 9
  GLYPH( _,_,_,   _,X,_,   _,_,_,   _,X,_,   _,_,_ ) // :
  GLYPH( _,_,_,   _,X,_,   _,_,_,   X,X,_,   _,X,_ ) // ;
  GLYPH( _,_,X,   _,X,_,   X,_,_,   _,X,_,   _,_,X ) // <
  GLYPH( _,_,_,   X,X,X,   _,_,_,   X,X,X,   _,_,_ ) // =
  GLYPH( X,_,_,   _,X,_,   _,_,X,   _,X,_,   X,_,_ ) // >
  GLYPH( X,X,_,   _,_,X,   _,X,X,   _,_,_,   _,X,_ ) // ?
  
  GLYPH( X,X,X,   X,_,X,   X,_,X,   X,_,_,   X,X,X ) // @
  GLYPH( _,X,_,   X,_,X,   X,X,X,   X,_,X,   X,_,X ) // A
  GLYPH( X,X,_,   X,_,X,   X,X,_,   X,_,X,   X,X,_ ) // B
  GLYPH( _,X,X,   X,_,_,   X,_,_,   X,_,_,   _,X,X ) // C
  GLYPH( X,X,_,   X,_,X,   X,_,X,   X,_,X,   X,X,_ ) // D
  GLYPH( X,X,X,   X,_,_,   X,X,_,   X,_,_,   X,X,X ) // E
  GLYPH( X,X,X,   X,_,_,   X,X,_,   X,_,_,   X,_,_ ) // F
  GLYPH( _,X,_,   X,_,_,   X,_,X,   X,_,X,   _,X,X ) // G
  GLYPH( X,_,X,   X,_,X,   X,X,X,   X,_,X,   X,_,X ) // H
  GLYPH( X,X,X,   _,X,_,   _,X,_,   _,X,_,   X,X,X ) // I
  GLYPH( _,_,X,   _,_,X,   _,_,X,   X,_,X,   _,X,_ ) // J
  GLYPH( X,_,X,   X,_,X,   X,X,_,   X,_,X,   X,_,X ) // K
  GLYPH( X,_,_,   X,_,_,   X,_,_,   X,_,_,   X,X,X ) // L
  GLYPH( X,X,X,   X,X,X,   X,X,X,   X,_,X,   X,_,X ) // M
  GLYPH( _,_,X,   X,X,X,   X,X,X,   X,X,X,   X,_,_ ) // N
  GLYPH( _,X,_,   X,_,X,   X,_,X,   X,_,X,   _,X,_ ) // O
  
  GLYPH( X,X,_,   X,_,X,   X,X,_,   X,_,_,   X,_,_ ) // P
  GLYPH( _,X,_,   X,_,X,   X,_,X,   X,X,X,   _,X,X ) // Q
  GLYPH( X,X,_,   X,_,X,   X,X,_,   X,_,X,   X,_,X ) // R
  GLYPH( _,X,X,   X,_,_,   _,X,_,   _,_,X,   X,X,_ ) // S
  GLYPH( X,X,X,   _,X,_,   _,X,_,   _,X,_,   _,X,_ ) // T
  GLYPH( X,_,X,   X,_,X,   X,_,X,   X,_,X,   X,X,X ) // U
  GLYPH( X,_,X,   X,_,X,   X,_,X,   X,X,X,   _,X,_ ) // V
  GLYPH( X,_,X,   X,_,X,   X,X,X,   X,X,X,   X,X,X ) // W
  GLYPH( X,_,X,   X,_,X,   _,X,_,   X,_,X,   X,_,X ) // X
  GLYPH( X,_,X,   X,_,X,   _,X,_,   _,X,_,   _,X,_ ) // Y
  GLYPH( X,X,X,   _,_,X,   _,X,_,   X,_,_,   X,X,X ) // Z
  GLYPH( _,X,X,   _,X,_,   _,X,_,   _,X,_,   _,X,X ) // [
  GLYPH( X,_,_,   X,_,_,   _,X,_,   _,_,X,   _,_,X ) // backslash
  GLYPH( X,X,_,   _,X,_,   _,X,_,   _,X,_,   X,X,_ ) // ]
  GLYPH( _,X,_,   X,_,X,   _,_,_,   _,_,_,   _,_,_ ) // ^
  GLYPH( _,_,_,   _,_,_,   _,_,_,   _,_,_,   X,X,X ) // _
  
  GLYPH( X,_,_,   _,X,_,   _,_,X,   _,_,_,   _,_,_ ) // `
  GLYPH( _,X,_,   X,_,X,   X,X,X,   X,_,X,   X,_,X ) // a
  GLYPH( X,X,_,   X,_,X,   X,X,_,   X,_,X,   X,X,_ ) // b
  GLYPH( _,X,X,   X,_,_,   X,_,_,   X,_,_,   _,X,X ) // c
  GLYPH( X,X,_,   X,_,X,   X,_,X,   X,_,X,   X,X,_ ) // d
  GLYPH( X,X,X,   X,_,_,   X,X,_,   X,_,_,   X,X,X ) // e
  GLYPH( X,X,X,   X,_,_,   X,X,_,   X,_,_,   X,_,_ ) // f
  GLYPH( _,X,_,   X,_,_,   X,_,X,   X,_,X,   _,X,X ) // g
  GLYPH( X,_,X,   X,_,X,   X,X,X,   X,_,X,   X,_,X ) // h
  GLYPH( X,X,X,   _,X,_,   _,X,_,   _,X,_,   X,X,X ) // i
  GLYPH( _,_,X,   _,_,X,   _,_,X,   X,_,X,   _,X,_ ) // j
  GLYPH( X,_,X,   X,_,X,   X,X,_,   X,_,X,   X,_,X ) // k
  GLYPH( X,_,_,   X,_,_,   X,_,_,   X,_,_,   X,X,X ) // l
  GLYPH( X,X,X,   X,X,X,   X,X,X,   X,_,X,   X,_,X ) // m
  GLYPH( _,_,X,   X,X,X,   X,X,X,   X,X,X,   X,_,_ ) // n
  GLYPH( _,X,_,   X,_,X,   X,_,X,   X,_,X,   _,X,_ ) // o
  
  GLYPH( X,X,_,   X,_,X,   X,X,_,   X,_,_,   X,_,_ ) // p
  GLYPH( _,X,_,   X,_,X,   X,_,X,   X,X,X,   _,X,X ) // q
  GLYPH( X,X,_,   X,_,X,   X,X,_,   X,_,X,   X,_,X ) // r
  GLYPH( _,X,X,   X,_,_,   _,X,_,   _,_,X,   X,X,_ ) // s
  GLYPH( X,X,X,   _,X,_,   _,X,_,   _,X,_,   _,X,_ ) // t
  GLYPH( X,_,X,   X,_,X,   X,_,X,   X,_,X,   X,X,X ) // u
  GLYPH( X,_,X,   X,_,X,   X,_,X,   X,X,X,   _,X,_ ) // v
  GLYPH( X,_,X,   X,_,X,   X,X,X,   X,X,X,   X,X,X ) // w
  GLYPH( X,_,X,   X,_,X,   _,X,_,   X,_,X,   X,_,X ) // x
  GLYPH( X,_,X,   X,_,X,   _,X,_,   _,X,_,   _,X,_ ) // y
  GLYPH( X,X,X,   _,_,X,   _,X,_,   X,_,_,   X,X,X ) // z
  GLYPH( _,_,X,   _,X,_,   X,X,_,   _,X,_,   _,_,X ) // {
  GLYPH( _,X,_,   _,X,_,   _,X,_,   _,X,_,   _,X,_ ) // |
  GLYPH( X,_,_,   _,X,_,   _,X,X,   _,X,_,   X,_,_ ) // }
  GLYPH( _,_,_,   _,X,_,   X,_,X,   _,X,_,   _,_,_ ) // ~
#undef GLYPH
#undef _
#undef X
};
 
static void draw_glyph_y8(uint8_t *dst,int dstw,int dsth,int dstx,int dsty,unsigned char src) {
  if ((dstx<0)||(dsty<0)||(dstx>dstw-3)||(dsty>dsth-5)) return; // No clipping. If it runs OOB, nix the whole glyph.
  if ((src<=0x20)||(src>=0x7f)) return; // G0 only.
  uint16_t srcbits=demo_glyphv_3x5[src-0x21];
  uint8_t *dstrow=dst+dsty*dstw+dstx;
  int yi=5; for (;yi-->0;dstrow+=dstw,srcbits<<=3) {
    if (srcbits&0x8000) dstrow[0]=0xff;
    if (srcbits&0x4000) dstrow[1]=0xff;
    if (srcbits&0x2000) dstrow[2]=0xff;
  }
}
 
static void draw_text_y8(uint8_t *dst,int dstw,int dsth,int dstx,int dsty,const char *src,int srcc) {
  if (!src) return;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  for (;srcc-->0;src++,dstx+=4) {
    if ((unsigned char)(*src)<=0x20) continue;
    draw_glyph_y8(dst,dstw,dsth,dstx,dsty,*src);
  }
}

/* Draw frame in Tiny bgr332.
 */
 
static void demo_draw_bgr332(uint8_t *dst,int dstw,int dsth) {
  memset(dst,0,dstw*dsth);
  char msg[256];
  int msgc=0;
  memcpy(msg+msgc,"in: ",4);
  msgc+=4;
  if (demo.instate&0x01) { memcpy(msg+msgc,"left ",5); msgc+=5; }
  if (demo.instate&0x02) { memcpy(msg+msgc,"right ",6); msgc+=6; }
  if (demo.instate&0x04) { memcpy(msg+msgc,"up ",3); msgc+=3; }
  if (demo.instate&0x08) { memcpy(msg+msgc,"down ",5); msgc+=4; }
  if (demo.instate&0x10) { memcpy(msg+msgc,"a ",2); msgc+=2; }
  if (demo.instate&0x20) { memcpy(msg+msgc,"b ",2); msgc+=2; }
  if (demo.instate&0x40) { memcpy(msg+msgc,"c ",2); msgc+=2; }
  if (demo.instate&0x80) { memcpy(msg+msgc,"d ",2); msgc+=2; }
  draw_text_y8(dst,dstw,dsth,0,0,msg,msgc);
  
  msgc=0;
  memcpy(msg+msgc,"fc: ",4);
  msgc+=4;
  if (demo.framec>=1000) msg[msgc++]='0'+((demo.framec/1000)%10);
  if (demo.framec>=100) msg[msgc++]='0'+((demo.framec/100)%10);
  if (demo.framec>=10) msg[msgc++]='0'+((demo.framec/10)%10);
  msg[msgc++]='0'+(demo.framec%10);
  draw_text_y8(dst,dstw,dsth,0,6,msg,msgc);
}

/* Draw frame, dispatch on format.
 */
 
static void demo_draw(uint8_t *dst,int dstw,int dsth,int dststride,int pixelsize,int pixfmt) {
  switch (pixfmt) {
    case 0x31: demo_draw_bgr332(dst,dstw,dsth); return;
  }
  switch (pixelsize) {
    //case 8: demo_draw_y8(dst,dstw,dsth); return;
  }
  memset(dst,0xff,dststride*dsth);
}

/* Main loop without using the helper library.
 * I wrote this initially while building the thing out, and it seems a shame to chuck it.
 * But in real life, you really ought to use the library instead.
 */
 
static int demo_main_libless(const char *devpath) {
  //TODO We need to initialize the device before opening:
  // stty -F /dev/ttyACM0 cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
  int fd=open(devpath,O_RDWR);
  if (fd<0) {
    fprintf(stderr,"%s: Open failed: %m\n",devpath);
    return 1;
  }
  fprintf(stderr,"%s: Opened device.\n",devpath);
  
  // Request framebuffer format and await response.
  uint8_t request=0x01;
  if (write(fd,&request,1)!=1) {
    fprintf(stderr,"%s: Write failed: %m\n",devpath);
    close(fd);
    return 1;
  }
  fprintf(stderr,"%s: Sent request for framebuffer format.\n",devpath);
  // If we're doing things right, we shouldn't assume that Declare Format is the next packet.
  // Device is free to send Input whenever it likes.
  uint8_t format[6]={0};
  if (read(fd,format,sizeof(format))!=sizeof(format)) {
    fprintf(stderr,"%s: Read failed: %m\n",devpath);
    close(fd);
    return 1;
  }
  fprintf(stderr,"%s: Got format: %02x %02x %02x %02x %02x %02x\n",devpath,format[0],format[1],format[2],format[3],format[4],format[5]);
  if (format[0]!=0x03) {
    fprintf(stderr,"%s: Expected Declare Format packet, found %02x %02x %02x %02x %02x %02x\n",devpath,format[0],format[1],format[2],format[3],format[4],format[5]);
    close(fd);
    return 1;
  }
  int fbw=(format[1]<<8)|format[2];
  int fbh=(format[3]<<8)|format[4];
  if ((fbw<1)||(fbh<1)||(fbw>1024)||(fbh>1024)) {
    fprintf(stderr,"%s: Invalid framebuffer size %dx%d\n",devpath,fbw,fbh);
    close(fd);
    return 1;
  }
  int pixfmt=format[5];
  int pixelsize;
  switch (pixfmt&0xf0) {
    case 0x00: pixelsize=1; break;
    case 0x10: pixelsize=2; break;
    case 0x20: pixelsize=4; break;
    case 0x30: pixelsize=8; break;
    case 0x40: pixelsize=16; break;
    case 0x50: pixelsize=32; break;
    case 0xf0: pixelsize=24; break;
    default: {
        fprintf(stderr,"%s: Illegal pixel format %#.2x\n",devpath,pixfmt);
        close(fd);
        return 1;
      }
  }
  int stride=(fbw*pixelsize+7)>>3;
  int fblen=stride*fbh;
  uint8_t *fb=calloc(stride,fblen);
  if (!fb) {
    close(fd);
    return 1;
  }
  fprintf(stderr,"%s: Format OK (%dx%d, pixelsize=%d, pixfmt=0x%02x). Begin main loop.\n",devpath,fbw,fbh,pixelsize,pixfmt);
  
  while (!demo.sigc) {
  
    struct pollfd pollfd={.fd=fd,.events=POLLIN|POLLERR|POLLHUP};
    if (poll(&pollfd,1,0)>0) {
      uint8_t buf[256];
      int bufc=read(fd,buf,sizeof(buf));
      if (bufc<=0) {
        fprintf(stderr,"%s: Connection closed.\n",devpath);
        break;
      }
      int bufp=0;
      while (bufp<bufc) {
        switch (buf[bufp++]) {
          case 0x00: { // Abort.
              fprintf(stderr,"%s: Explicit abort.\n",devpath);
              goto _done_;
            }
          case 0x03: { // Declare format. We've already seen it once, and it's not allowed to change.
              bufp+=5;
            } break;
          case 0x04: { // Input State.
              demo.instate=buf[bufp++];
            } break;
        }
      }
    }
    
    demo_draw(fb,fbw,fbh,stride,pixelsize,pixfmt);
    uint8_t fbopcode=0x02;
    if (write(fd,&fbopcode,1)<0) {
      fprintf(stderr,"%s: Write failed: %m\n",devpath);
      close(fd);
      return 1;
    }
    if (write(fd,fb,fblen)!=fblen) {
      fprintf(stderr,"%s: Write failed: %m\n",devpath);
      close(fd);
      return 1;
    }
  
    demo.framec++;
    usleep(200000); // Aim for about 5 Hz, doesn't need to be precise.
  }
 _done_:;
  close(fd);
  return 0;
}

/* Main loop using libgamemon.
 * This is what a real game should do.
 */
 
static void cb_connected(void *userdata) {
  fprintf(stderr,"%s\n",__func__);
}
 
static void cb_disconnected(void *userdata) {
  fprintf(stderr,"%s\n",__func__);
}
 
static void cb_fb_format(int w,int h,int pixfmt,void *userdata) {
  fprintf(stderr,"%s %dx%d 0x%02x\n",__func__,w,h,pixfmt);
  int len=gamemon_measure_framebuffer(w,h,pixfmt);
  if (len<1) return;
  void *nv=malloc(len);
  if (!nv) return;
  if (demo.fb) free(demo.fb);
  demo.fb=nv;
  demo.fbw=w;
  demo.fbh=h;
  demo.fbfmt=pixfmt;
  demo.fblen=len;
}
 
static void cb_input(int state,int pvstate,void *userdata) {
  fprintf(stderr,"%s 0x%02x (prev 0x%02x)\n",__func__,state,pvstate);
  demo.instate=state;
}
 
static int demo_main_libful(const char *devpath,int baud_rate) {

  struct gamemon_delegate delegate={
    .connected=cb_connected,
    .disconnected=cb_disconnected,
    .fb_format=cb_fb_format,
    .input=cb_input,
  };
  if (!(demo.gamemon=gamemon_new(&delegate,devpath,baud_rate))) {
    fprintf(stderr,"gamemon_new failed. devpath='%s', baud_rate=%d\n",devpath,baud_rate);
    return 1;
  }
  
  while (!demo.sigc) {
    if (gamemon_update(demo.gamemon)<0) {
      fprintf(stderr,"gamemon_update failed\n");
      return 1;
    }
    if (demo.fbfmt==GAMEMON_PIXFMT_BGR332) {
      demo_draw(demo.fb,demo.fbw,demo.fbh,demo.fbw,8,demo.fbfmt);
      gamemon_send_framebuffer(demo.gamemon,demo.fbw,demo.fbh,demo.fbfmt,demo.fb,demo.fblen);
    }
    demo.framec++;
    usleep(200000);
  }

  return 0;
}

/* Main.
 */
 
int main(int argc,char **argv) {

  signal(SIGINT,demo_rcvsig);

  int status=1,argi=1,libless=0,baud_rate=0;
  const char *devpath=0;
  for (;argi<argc;argi++) {
    const char *arg=argv[argi];
    if (!strcmp(arg,"--help")) {
      fprintf(stdout,"Usage: %s [--libless] [--baud_rate=INT] DEVICE\n",argv[0]);
      return 0;
    }
    if (!strcmp(arg,"--libless")) {
      libless=1;
      continue;
    }
    if (!memcmp(arg,"--baud_rate=",12)) {
      arg+=12;
      for (;*arg;arg++) {
        baud_rate*=10;
        baud_rate+=(*arg)-'0';
      }
      continue;
    }
    if ((arg[0]=='-')||devpath) {
      fprintf(stderr,"%s: Unexpected argument '%s'\n",argv[0],arg);
      return 1;
    }
    devpath=arg;
  }
  if (!devpath) {
    fprintf(stderr,"%s: Device path required\n",argv[0]);
    return 1;
  }
  
  if (libless) status=demo_main_libless(devpath);
  else status=demo_main_libful(devpath,baud_rate);
  
  gamemon_del(demo.gamemon);
  return status;
}
