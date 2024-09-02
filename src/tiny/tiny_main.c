void tiny_init();
int tiny_update(); // =>(LEFT,RIGHT,UP,DOWN,A,B)
void tiny_send_framebuffer(const void *fb/*96x64xbgr332*/);
void usb_begin();
void usb_send(const void *v,int c);
int usb_read(void *dst,int dsta);
int usb_read_byte();

static unsigned char pvinput=0;
static int running=1;
static unsigned char fb[96*64];

void setup() {
  tiny_init();
  usb_begin();
  memset(fb,0x92,sizeof(fb));
  tiny_send_framebuffer(fb);
}

static int failure_code=0;

static void fill_rect(unsigned char *dstrow,int w,int h,int flag) {
  unsigned char color=flag?0xff:0x00;
  for (;h-->0;dstrow+=96) {
    unsigned char *dstp=dstrow;
    int xi=w;
    for (;xi-->0;dstp++) *dstp=color;
  }
}

static void write_failure_message(unsigned char m) {
  unsigned char *dst=fb+96*25+20;
  fill_rect(dst,2,4,m&0x80); dst+=3;
  fill_rect(dst,2,4,m&0x40); dst+=3;
  fill_rect(dst,2,4,m&0x20); dst+=3;
  fill_rect(dst,2,4,m&0x10); dst+=3;
  fill_rect(dst,2,4,m&0x08); dst+=3;
  fill_rect(dst,2,4,m&0x04); dst+=3;
  fill_rect(dst,2,4,m&0x02); dst+=3;
  fill_rect(dst,2,4,m&0x01); dst+=3;
}

void loop() {
  if (!running) {
    fb[20*96+20]=0xff;
    fb[20*96+22]=failure_code;
    tiny_send_framebuffer(fb);
    delay(10000);
    return;
  }
  unsigned char input=tiny_update();
  if (input!=pvinput) {
    unsigned char packet[]={0x04,input};
    usb_send(packet,sizeof(packet));
    pvinput=input;
  }
  int opcode=usb_read_byte();
  if (opcode<0) return;
  switch (opcode) {
    case 0x00: { // Abort.
        write_failure_message(0);
        running=0;
      } break;
    case 0x01: { // Request format.
        unsigned char response[]={
          0x03,
          0,96,
          0,64,
          0x31, // bgr332
        };
        usb_send(response,sizeof(response));
      } break;
    case 0x02: { // Framebuffer.
        unsigned char fb[96*64];
        if (usb_read(fb,sizeof(fb))>=sizeof(fb)) {
          tiny_send_framebuffer(fb);
        }
      } break;
    default: { // Unknown opcode. Abort.
        failure_code=opcode;
        write_failure_message(opcode);
        unsigned char abort=0x00;
        usb_send(&abort,1);
        running=0;
      }
  }
}
