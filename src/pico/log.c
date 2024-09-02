#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>

static uint16_t *fb=0;
static int fbw=0,fbh=0;
static int dsty=0;

/* Init.
 */
 
void log_init(void *_fb,int _fbw,int _fbh) {
  fb=_fb;
  fbw=_fbw;
  fbh=_fbh;
}

/* Draw one glyph.
 */
static const uint16_t glyphv_3x5[95]={
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
 
static void draw_glyph(int dstx,int dsty,char ch) {
  if ((ch<0x21)||(ch>=0x7f)) return;
  if ((dstx<0)||(dsty<0)||(dstx>fbw-3)||(dsty>fbh-5)) return; // No clipping. We draw the whole glyph or nothing.
  uint16_t srcbits=glyphv_3x5[ch-0x21];
  uint16_t *row=fb+dsty*fbw+dstx;
  int yi=5; for (;yi-->0;row+=fbw,srcbits<<=3) {
    if (srcbits&0x8000) row[0]=0xffff;
    if (srcbits&0x4000) row[1]=0xffff;
    if (srcbits&0x2000) row[2]=0xffff;
  }
}

/* Log raw text, plus a newline.
 */
 
static void log_text(const char *src,int srcc) {
  if (dsty>fbh-6) {
    memmove(fb,fb+fbw*6,fbw*2*(fbh-6));
    memset(fb+fbw*(fbh-6),0,fbw*2*6);
    dsty-=6;
  }
  int dstx=0;
  for (;srcc-->0;src++,dstx+=4) draw_glyph(dstx,dsty,*src);
  dsty+=6;
}

/* Log formatted text.
 */
 
void log_fmt(const char *fmt,...) {
  if (!fb||(fbw<1)||(fbh<1)) return;
  char msg[256];
  va_list vargs;
  va_start(vargs,fmt);
  int msgc=0;
  while (*fmt&&(msgc<sizeof(msg))) {
    if (*fmt!='%') {
      msg[msgc++]=*fmt;
      fmt++;
    } else if (fmt[1]=='%') {
      msg[msgc++]='%';
      fmt+=2;
    } else {
      fmt+=2; // We don't care what the format unit actually is, we assume "%d". And if there's width or flags, all hell breaks loose.
      int v=va_arg(vargs,int);
      int digitc=1,limit=10;
      while (v>=limit) { digitc++; if (limit>INT_MAX/10) break; limit*=10; }
      if (msgc>sizeof(msg)-digitc) break;
      int i=digitc; for (;i-->0;v/=10) msg[msgc+i]='0'+v%10;
      msgc+=digitc;
    }
  }
  log_text(msg,msgc);
}
