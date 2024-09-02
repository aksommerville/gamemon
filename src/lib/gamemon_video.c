#include "gamemon_internal.h"

/* Pixel size in bits.
 */
 
int gamemon_pixel_size(int pixfmt) {
  if (pixfmt&~0xff) return 0;
  switch (pixfmt&0xf0) {
    case 0x00: return 1;
    case 0x10: return 2;
    case 0x20: return 4;
    case 0x30: return 8;
    case 0x40: return 16;
    case 0x50: return 32;
    case 0xf0: return 24;
  }
  return 0;
}

/* Framebuffer size in bytes.
 */

int gamemon_measure_framebuffer(int w,int h,int pixfmt) {
  if ((w<1)||(w>GAMEMON_FRAMEBUFFER_SIZE_LIMIT)) return 0;
  if ((h<1)||(h>GAMEMON_FRAMEBUFFER_SIZE_LIMIT)) return 0;
  int pixelsize=gamemon_pixel_size(pixfmt);
  if (pixelsize<1) return 0;
  int stride=(w*pixelsize+7)>>3;
  return stride*h;
}

/* Abstract pixel accessors.
 * We only care about the pixels' size.
 */
 
typedef int (*gamemon_pxrd_fn)(const uint8_t *v,int x);
typedef void (*gamemon_pxwr_fn)(uint8_t *v,int x,int pixel);

static int gamemon_pxrd_dummy(const uint8_t *v,int x) {
  return 0;
}
static void gamemon_pxwr_dummy(uint8_t *v,int x,int pixel) {
}

static int gamemon_pxrd_1(const uint8_t *v,int x) {
  return (v[x>>3]&(0x80>>(x&7)))?1:0;
}
static void gamemon_pxwr_1(uint8_t *v,int x,int pixel) {
  if (pixel&1) v[x>>3]|=0x80>>(x&7);
  else v[x>>3]&=~(0x80>>(x&7));
}

static int gamemon_pxrd_2(const uint8_t *v,int x) {
  return (v[x>>2]>>((3-(x&3))<<1))&3;
}
static void gamemon_pxwr_2(uint8_t *v,int x,int pixel) {
  int shift=(3-(x&3))<<1;
  pixel=(pixel&3)<<shift;
  v[x>>2]=(v[x>>2]&~(3<<shift))|pixel;
}

static int gamemon_pxrd_4(const uint8_t *v,int x) {
  if (x&1) return v[x>>1]&15;
  return v[x>>1]>>4;
}
static void gamemon_pxwr_4(uint8_t *v,int x,int pixel) {
  if (x&1) v[x>>1]=(v[x>>1]&0xf0)|(pixel&0x0f);
  else v[x>>1]=(v[x>>1]&0x0f)|(pixel<<4);
}

static int gamemon_pxrd_8(const uint8_t *v,int x) {
  return v[x];
}
static void gamemon_pxwr_8(uint8_t *v,int x,int pixel) {
  v[x]=pixel;
}

static int gamemon_pxrd_16(const uint8_t *v,int x) {
  v+=x<<1;
  return (v[0]<<8)|v[1];
}
static void gamemon_pxwr_16(uint8_t *v,int x,int pixel) {
  v+=x<<1;
  v[0]=x>>8;
  v[1]=x;
}

static int gamemon_pxrd_24(const uint8_t *v,int x) {
  v+=x*3;
  return (v[0]<<16)|(v[1]<<8)|v[2];
}
static void gamemon_pxwr_24(uint8_t *v,int x,int pixel) {
  v+=x*3;
  v[0]=pixel>>16;
  v[1]=pixel>>8;
  v[2]=pixel;
}

static int gamemon_pxrd_32(const uint8_t *v,int x) {
  v+=x<<2;
  return (v[0]<<24)|(v[1]<<16)|(v[2]<<8)|v[3];
}
static void gamemon_pxwr_32(uint8_t *v,int x,int pixel) {
  v+=x<<2;
  v[0]=pixel>>24;
  v[1]=pixel>>16;
  v[2]=pixel>>8;
  v[3]=pixel;
}

static gamemon_pxrd_fn gamemon_pxrd_get(int fmt) {
  switch (gamemon_pixel_size(fmt)) {
    case 1: return gamemon_pxrd_1;
    case 2: return gamemon_pxrd_2;
    case 4: return gamemon_pxrd_4;
    case 8: return gamemon_pxrd_8;
    case 16: return gamemon_pxrd_16;
    case 24: return gamemon_pxrd_24;
    case 32: return gamemon_pxrd_32;
  }
  return gamemon_pxrd_dummy;
}

static gamemon_pxwr_fn gamemon_pxwr_get(int fmt) {
  switch (gamemon_pixel_size(fmt)) {
    case 1: return gamemon_pxwr_1;
    case 2: return gamemon_pxwr_2;
    case 4: return gamemon_pxwr_4;
    case 8: return gamemon_pxwr_8;
    case 16: return gamemon_pxwr_16;
    case 24: return gamemon_pxwr_24;
    case 32: return gamemon_pxwr_32;
  }
  return gamemon_pxwr_dummy;
}

/* Abstract pixel converter.
 */
 
typedef int (*gamemon_pxcvt_fn)(int src);

static int gamemon_pxcvt_dummy(int src) {
  return src;
}

static int gamemon_pxcvt_rgba_y1be(int src) {
  return (src&1)?0xffffffff:0x000000ff;
}
static int gamemon_pxcvt_y1be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  return (r+g+b>=384)?1:0;
}

static int gamemon_pxcvt_rgba_y2be(int src) {
  int luma=src&3;
  luma|=luma<<2;
  luma|=luma<<4;
  return (luma<<24)|(luma<<16)|(luma<<8)|0xff;
}
static int gamemon_pxcvt_y2be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  int luma=(r+g+b)/3;
  return luma>>6;
}

static int gamemon_pxcvt_rgba_y4be(int src) {
  int luma=src&16;
  luma|=luma<<4;
  return (luma<<24)|(luma<<16)|(luma<<8)|0xff;
}
static int gamemon_pxcvt_y4be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  int luma=(r+g+b)/3;
  return luma>>4;
}

static const int gamemon_cga4be[]={
  0x000000ff,0x0000AAff,0x00AA00ff,0x00AAAAff,
  0xAA0000ff,0xAA00AAff,0xAA5500ff,0xAAAAAAff,
  0x555555ff,0x5555FFff,0x55FF55ff,0x55FFFFff,
  0xFF5555ff,0xFF55FFff,0xFFFF55ff,0xFFFFFFff,
};
static int gamemon_pxcvt_rgba_cga4be(int src) {
  return gamemon_cga4be[src&15];
}
static int gamemon_pxcvt_cga4be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  int best=0,bestscore=768;
  const int *ctab=gamemon_cga4be;
  int i=16;
  for (;i-->0;ctab++) {
    uint8_t qr=(*ctab)>>24,qg=(*ctab)>>16,qb=(*ctab)>>8;
    int dr=qr-r; if (dr<0) dr=-dr;
    int dg=qg-g; if (dg<0) dg=-dg;
    int db=qb-b; if (db<0) db=-db;
    int score=dr+dg+db;
    if (score<bestscore) { best=*ctab; bestscore=score; }
  }
  return best;
}

static int gamemon_pxcvt_rgba_y8(int src) {
  return (src<<24)|(src<<16)|(src<<8)|0xff;
}
static int gamemon_pxcvt_y8_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  return (r+g+b)/3;
}

static int gamemon_pxcvt_rgba_bgr332(int src) {
  uint8_t r=src&0x03; r|=r<<2; r|=r<<4;
  uint8_t g=src&0x1c; g<<=3; g|=g>>3; g|=g>>6;
  uint8_t b=src&0xe0; b|=b>>3; b|=b>>6;
  return (r<<24)|(g<<16)|(b<<8)|0xff;
}
static int gamemon_pxcvt_bgr332_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  return (b&0xe0)|((g>>3)&0x1c)|(r>>6);
}

static int gamemon_pxcvt_rgba_y16be(int src) {
  uint8_t luma=src>>8;
  return (luma<<24)|(luma<<16)|(luma<<8)|0xff;
}
static int gamemon_pxcvt_y16be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  uint8_t luma=(r+g+b)/3;
  return (luma<<8)|luma;
}

static int gamemon_pxcvt_rgba_xrgb4444be(int src) {
  uint8_t r=(src>>8)&15; r|=r<<4;
  uint8_t g=src&0xf0; g|=g>>4;
  uint8_t b=src&0x0f; b|=b<<4;
  return (r<<24)|(g<<16)|(b<<8)|0xff;
}
static int gamemon_pxcvt_xrgb4444be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  return 0xf000|((r&0xf0)<<4)|(g&0xf0)|(b>>4);
}

static int gamemon_pxcvt_rgba_bgr565be(int src) {
  uint8_t b=(src>>8)&0xf8; b|=b>>5;
  uint8_t g=(src>>3)&0xfc; g|=g>>6;
  uint8_t r=src<<3; r|=r>>5;
  return (r<<24)|(g<<16)|(b<<8)|0xff;
}
static int gamemon_pxcvt_bgr565be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  return ((b&0xf8)<<8)|((g&0xfc)<<3)|(r>>3);
}

static int gamemon_pxcvt_rgba_y32be(int src) {
  uint8_t luma=src>>24;
  return (luma<<24)|(luma<<16)|(luma<<8)|0xff;
}
static int gamemon_pxcvt_y32be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  uint8_t luma=(r+g+b)/3;
  return (luma<<24)|(luma<<16)|(luma<<8)|luma;
}

static int gamemon_pxcvt_rgba_bgrx(int src) {
  uint8_t b=src>>24,g=src>>16,r=src>>8;
  return (r<<24)|(g<<16)|(b<<8)|0xff;
}
static int gamemon_pxcvt_bgrx_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  return (b<<24)|(g<<16)|(r<<8)|0xff;
}

static int gamemon_pxcvt_rgba_xrgb(int src) {
  uint8_t r=src>>16,g=src>>8,b=src;
  return (r<<24)|(g<<16)|(b<<8)|0xff;
}
static int gamemon_pxcvt_xrgb_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  return (r<<16)|(g<<8)|b;
}

static int gamemon_pxcvt_rgba_xbgr(int src) {
  uint8_t b=src>>16,g=src>>8,r=src;
  return (r<<24)|(g<<16)|(b<<8)|0xff;
}
static int gamemon_pxcvt_xbgr_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  return (b<<16)|(g<<8)|r;
}

static int gamemon_pxcvt_rgba_y24be(int src) {
  uint8_t luma=src>>16;
  return (luma<<24)|(luma<<16)|(luma<<8)|0xff;
}
static int gamemon_pxcvt_y24be_rgba(int src) {
  uint8_t r=src>>24,g=src>>16,b=src>>8;
  uint8_t luma=(r+g+b)/3;
  return (luma<<16)|(luma<<8)|luma;
}

static gamemon_pxcvt_fn gamemon_pxcvt_to_rgba(int fmt) {
  switch (fmt) {
    case GAMEMON_PIXFMT_Y1BE: return gamemon_pxcvt_rgba_y1be;
    case GAMEMON_PIXFMT_Y2BE: return gamemon_pxcvt_rgba_y2be;
    case GAMEMON_PIXFMT_Y4BE: return gamemon_pxcvt_rgba_y4be;
    case GAMEMON_PIXFMT_CGA4BE: return gamemon_pxcvt_rgba_cga4be;
    case GAMEMON_PIXFMT_Y8: return gamemon_pxcvt_rgba_y8;
    case GAMEMON_PIXFMT_BGR332: return gamemon_pxcvt_rgba_bgr332;
    case GAMEMON_PIXFMT_Y16BE: return gamemon_pxcvt_rgba_y16be;
    case GAMEMON_PIXFMT_XRGB4444BE: return gamemon_pxcvt_rgba_xrgb4444be;
    case GAMEMON_PIXFMT_BGR565BE: return gamemon_pxcvt_rgba_bgr565be;
    case GAMEMON_PIXFMT_Y32BE: return gamemon_pxcvt_rgba_y32be;
    case GAMEMON_PIXFMT_RGBX: return gamemon_pxcvt_dummy;
    case GAMEMON_PIXFMT_BGRX: return gamemon_pxcvt_rgba_bgrx;
    case GAMEMON_PIXFMT_XRGB: return gamemon_pxcvt_rgba_xrgb;
    case GAMEMON_PIXFMT_XBGR: return gamemon_pxcvt_rgba_xbgr;
    case GAMEMON_PIXFMT_Y24BE: return gamemon_pxcvt_rgba_y24be;
    case GAMEMON_PIXFMT_RGB: return gamemon_pxcvt_rgba_xrgb;
    case GAMEMON_PIXFMT_BGR: return gamemon_pxcvt_rgba_xbgr;
  }
  return gamemon_pxcvt_dummy;
}

static gamemon_pxcvt_fn gamemon_pxcvt_from_rgba(int fmt) {
  switch (fmt) {
    case GAMEMON_PIXFMT_Y1BE: return gamemon_pxcvt_y1be_rgba;
    case GAMEMON_PIXFMT_Y2BE: return gamemon_pxcvt_y2be_rgba;
    case GAMEMON_PIXFMT_Y4BE: return gamemon_pxcvt_y4be_rgba;
    case GAMEMON_PIXFMT_CGA4BE: return gamemon_pxcvt_cga4be_rgba;
    case GAMEMON_PIXFMT_Y8: return gamemon_pxcvt_y8_rgba;
    case GAMEMON_PIXFMT_BGR332: return gamemon_pxcvt_bgr332_rgba;
    case GAMEMON_PIXFMT_Y16BE: return gamemon_pxcvt_y16be_rgba;
    case GAMEMON_PIXFMT_XRGB4444BE: return gamemon_pxcvt_xrgb4444be_rgba;
    case GAMEMON_PIXFMT_BGR565BE: return gamemon_pxcvt_bgr565be_rgba;
    case GAMEMON_PIXFMT_Y32BE: return gamemon_pxcvt_y32be_rgba;
    case GAMEMON_PIXFMT_RGBX: return gamemon_pxcvt_dummy;
    case GAMEMON_PIXFMT_BGRX: return gamemon_pxcvt_bgrx_rgba;
    case GAMEMON_PIXFMT_XRGB: return gamemon_pxcvt_xrgb_rgba;
    case GAMEMON_PIXFMT_XBGR: return gamemon_pxcvt_xbgr_rgba;
    case GAMEMON_PIXFMT_Y24BE: return gamemon_pxcvt_y24be_rgba;
    case GAMEMON_PIXFMT_RGB: return gamemon_pxcvt_xrgb_rgba;
    case GAMEMON_PIXFMT_BGR: return gamemon_pxcvt_xbgr_rgba;
  }
  return gamemon_pxcvt_dummy;
}

/* Convert framebuffer, anything to anything.
 * We're not going to scale. Copy as much as we can, centered.
 */
 
int gamemon_fb_convert(
  void *dst,int dsta,int dstw,int dsth,int dstfmt,
  const void *src,int srcc,int srcw,int srch,int srcfmt
) {
  int dststride=gamemon_measure_framebuffer(dstw,1,dstfmt);
  int srcstride=gamemon_measure_framebuffer(srcw,1,srcfmt);
  if ((dststride<1)||(srcstride<1)) return -1;
  int dstlen=dststride*dsth;
  int srclen=srcstride*srch;
  if ((dstlen>dsta)||(srclen>srcc)) return -1;
  
  // If either dst dimension smaller than src, black out dst first.
  if ((dstw<srcw)||(dsth<srch)) memset(dst,0,dstlen);
  
  // Decide how much we're copying per axis.
  int cpw=(dstw<srcw)?dstw:srcw;
  int cph=(dsth<srch)?dsth:srch;
  int dstx=(dstw>>1)-(cpw>>1);
  int dsty=(dsth>>1)-(cph>>1);
  int srcx=(srcw>>1)-(cpw>>1);
  int srcy=(srch>>1)-(cph>>1);
  
  // Iterate rows generically, and enter callbacks for access within the row, and conversion.
  uint8_t *dstrow=((uint8_t*)dst)+dsty*dststride;
  const uint8_t *srcrow=((uint8_t*)src)+srcy*srcstride;
  int yi=cph;
  if (dstfmt==srcfmt) {
    int pixelsize=gamemon_pixel_size(dstfmt);
    int cpc=(pixelsize+7)>>3;
    for (;yi-->0;dstrow+=dststride,srcrow+=srcstride) {
      memcpy(dstrow,srcrow,cpc);
    }
  } else {
    gamemon_pxwr_fn pxwr=gamemon_pxwr_get(dstfmt);
    gamemon_pxrd_fn pxrd=gamemon_pxrd_get(srcfmt);
    gamemon_pxcvt_fn dstcvt=gamemon_pxcvt_from_rgba(dstfmt);
    gamemon_pxcvt_fn srccvt=gamemon_pxcvt_to_rgba(srcfmt);
    for (;yi-->0;dstrow+=dststride,srcrow+=srcstride) {
      int dstxp=dstx,srcxp=srcx;
      int xi=cpw;
      for (;xi-->0;dstxp++,srcxp++) {
        pxwr(dstrow,dstxp,dstcvt(srccvt(pxrd(srcrow,srcxp))));
      }
    }
  }
  
  return dstlen;
}
