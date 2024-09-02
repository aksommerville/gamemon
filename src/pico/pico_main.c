#include "akpico.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "class/cdc/cdc_device.h"

//XXX External log unit, only using during initial troubleshooting.
void log_init(void *fb,int fbw,int fbh);
void log_fmt(const char *fmt,...);

static uint16_t *fb=0;
static int fbw=0,fbh=0,fbpixelsize=0;
static uint8_t input=0,pvinput=0;

/* tinyusb callbacks.
 * Copied and lightly massaged from TinyUSB/examples/device/cdc_msc.
 */
 
#define USB_BCD 0x0200
#define USB_VID 0xfffa
#define USB_PID 0x0001
 
#define EPNUM_CDC_NOTIF   0x81
#define EPNUM_CDC_OUT     0x02
#define EPNUM_CDC_IN      0x82

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

enum
{
  ITF_NUM_CDC = 0,
  ITF_NUM_CDC_DATA,
  ITF_NUM_MSC,
  ITF_NUM_TOTAL
};

static tusb_desc_device_t const desc_device =
{
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = USB_BCD,

  // Use Interface Association Descriptor (IAD) for CDC
  // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
  .bDeviceClass       = TUSB_CLASS_MISC,
  .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
  .bDeviceProtocol    = MISC_PROTOCOL_IAD,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor           = USB_VID,
  .idProduct          = USB_PID,
  .bcdDevice          = 0x0100,

  .iManufacturer      = 0x01,
  .iProduct           = 0x02,
  .iSerialNumber      = 0x03,

  .bNumConfigurations = 0x01
};

static const uint8_t config_descriptor_lowspeed[]={
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

  // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

static uint8_t const config_descriptor_highspeed[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

  // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),
};

// other speed configuration
static uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

// device qualifier is mostly similar to device descriptor since we don't change configuration based on speed
tusb_desc_device_qualifier_t const desc_device_qualifier =
{
  .bLength            = sizeof(tusb_desc_device_qualifier_t),
  .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
  .bcdUSB             = USB_BCD,

  .bDeviceClass       = TUSB_CLASS_MISC,
  .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
  .bDeviceProtocol    = MISC_PROTOCOL_IAD,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .bNumConfigurations = 0x01,
  .bReserved          = 0x00
};

// Invoked when received GET DEVICE QUALIFIER DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.
// device_qualifier descriptor describes information about a high-speed capable device that would
// change if the device were operating at the other speed. If not highspeed capable stall this request.
uint8_t const* tud_descriptor_device_qualifier_cb(void)
{
  return (uint8_t const*) &desc_device_qualifier;
}

// Invoked when received GET OTHER SEED CONFIGURATION DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
// Configuration descriptor in the other speed e.g if high speed then this is for full speed and vice versa
uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

  // if link speed is high return fullspeed config, and vice versa
  // Note: the descriptor type is OHER_SPEED_CONFIG instead of CONFIG
  memcpy(desc_other_speed_config,
         (tud_speed_get() == TUSB_SPEED_HIGH) ? config_descriptor_highspeed : config_descriptor_lowspeed,
         CONFIG_TOTAL_LEN);

  desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

  return desc_other_speed_config;
}

static const uint16_t string_descriptor_empty[]={
  0,0,
};
 
uint8_t const * tud_descriptor_device_cb(void) {
  return (uint8_t*)&desc_device;
}

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
  // Although we are highspeed, host may be fullspeed.
  return (tud_speed_get() == TUSB_SPEED_HIGH) ?  config_descriptor_highspeed : config_descriptor_lowspeed;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  return string_descriptor_empty;
}

/* USB CDC I/O.
 */
 
static void comms_init() {
  //board_init();
  tusb_init();
}
 
static int send_packet(const void *src,int srcc) {
  int err=tud_cdc_write(src,srcc);
  tud_cdc_write_flush();
  return err;
}

static volatile int notified=0;
static uint8_t *rbuf=0;
static volatile int rbufc=0;
static volatile int rbufa=0;

#if 0
static int recv_buffer(void *dst,int dstc) {
  //if (!notified) return 0;
  //notified=0;
  int dstp=0,panic=4;
  while (dstp<dstc) {
    int available=tud_cdc_available();
    log_fmt("  available=%d dst=%d/%d",available,dstp,dstc);
    if (available>0) {
      if (available>dstc-dstp) available=dstc-dstp;
      int err=tud_cdc_read((char*)dst+dstp,available);
      if (err<=0) break;
      dstp+=err;
    } else if (--panic<=0) {
      break;
    } else {
      sleep_ms(10);
    }
  }
  //tud_cdc_read_flush();
  return dstp;
}

static int recv_byte() {
  uint8_t tmp=0;
  if (recv_buffer(&tmp,1)<1) return -1;
  return tmp;
}
#endif
 
void tud_cdc_rx_cb(uint8_t itf) {
  notified=1;
  int available=tud_cdc_available();
  //log_fmt(__func__);
  //log_fmt("available=%d",available);
  int cpc=rbufa-rbufc;
  if (cpc>available) cpc=available;
  if (cpc<1) return;
  int err=tud_cdc_read(rbuf+rbufc,cpc);
  if (err>0) rbufc+=err;
  //log_fmt("rbuf: %d/%d (+%d)",rbufc,rbufa,err);
}

/* Main.
 */
 
//uint16_t altfb[240*240];
#define FAKEFBW 96
#define FAKEFBH 64
static uint8_t fakefb[FAKEFBW*FAKEFBH];

static void copyfb() {
  memset(fb,0,fbw*fbh*2);
  int dstx=(fbw>>1)-(FAKEFBW>>1);
  int dsty=(fbh>>1)-(FAKEFBH>>1);
  uint16_t *dstrow=fb+dsty*fbw+dstx;
  const uint8_t *srcrow=fakefb;
  int yi=FAKEFBH;
  for (;yi-->0;dstrow+=fbw,srcrow+=FAKEFBW) {
    uint16_t *dstp=dstrow;
    const uint8_t *srcp=srcrow;
    int xi=FAKEFBW;
    for (;xi-->0;dstp++,srcp++) {
      uint8_t r=(*srcp)&3; r|=r<<2;
      uint8_t g=((*srcp)&0x1c)>>1; g|=g>>3;
      uint8_t b=((*srcp)&0xe0)>>4; b|=b>>3;
      *dstp=(g<<12)|(b<<8)|r;
    }
  }
}

void main() {
  akpico_init();
  fb=akpico_get_framebuffer(&fbw,&fbh,&fbpixelsize);
  memset(fb,0,fbw*fbh*2);
  log_init(fb,fbw,fbh);
  //fbw=fbh=120;//XXX Lie about our dimensions, pretend very small. The real size, 240x240, seems to be problematically large.
  int fblen=fbw*fbh*2;
  comms_init();
  log_fmt("PicoSystem Gamemon v%d.%d.%d",1,2,3);
  log_fmt("We should emit text sensibly.");
  log_fmt("Begin main loop...");
  uint8_t instorage[16];
  uint8_t opcode=0;
  rbuf=&opcode;
  rbufc=0;
  rbufa=1;
  while (1) {
    akpico_update();
    tud_task();
    
    input=akpico_get_buttons();
    if (input!=pvinput) {
      const uint8_t pkt[]={0x04,input};
      //log_fmt("Send input %d",input);
      send_packet(pkt,sizeof(pkt));
      pvinput=input;
    }
    
    if (rbufc) {
      if (rbuf==&opcode) {
        rbufc=0;
        switch (opcode) {
          case 0x00: { // Abort.
              log_fmt("Abort per peer");
              akpico_send_framebuffer();
              return;
            }
          case 0x01: { // Request Format.
              const uint8_t pkt[]={
                0x03,
                //fbw>>8,fbw,
                //fbh>>8,fbh,
                //0x41, // xrgb4444be
                FAKEFBW>>8,FAKEFBW,
                FAKEFBH>>8,FAKEFBH,
                0x31, // bgr332
              };
              if (send_packet(pkt,sizeof(pkt))==sizeof(pkt)) {
                //log_fmt("Sent FB Format ok");
              } else {
                //log_fmt("Failed to send FB Format");
              }
              rbufa=1;
            } break;
          case 0x02: { // Framebuffer.
              //rbuf=(uint8_t*)altfb;
              //rbuf=(uint8_t*)fb;
              //rbufa=fblen;
              rbuf=fakefb;
              rbufa=FAKEFBW*FAKEFBH;
            } break;
          default: {
              log_fmt("Unexpected opcode %d",opcode);
              akpico_send_framebuffer();
              return;
            }
        }
      //} else if (rbuf==(void*)altfb) {
      } else if (rbuf==(void*)fakefb) {
        if (rbufc>=FAKEFBW*FAKEFBH) {
          rbuf=&opcode;
          rbufc=0;
          rbufa=1;
          copyfb();
          //log_fmt("Received framebuffer");
        }
      }
    }
    
    /*
    int opcode;
    while ((opcode=recv_byte())>=0) {
      log_fmt("Received opcode %d",opcode);
      switch (opcode) {
      
        case 0x00: { // Abort.
            log_fmt("Abort per peer");
            akpico_send_framebuffer();
            return;
          }
          
        case 0x01: { // Request Format.
            const uint8_t pkt[]={
              0x03,
              fbw>>8,fbw,
              fbh>>8,fbh,
              0x41, // xrgb4444be
            };
            if (send_packet(pkt,sizeof(pkt))==sizeof(pkt)) {
              log_fmt("Sent FB Format ok");
            } else {
              log_fmt("Failed to send FB Format");
            }
          } break;
          
        case 0x02: { // Framebuffer.
            int fblen=fbw*fbh*2;
            int rcvc=recv_buffer(altfb,fblen);
            if (rcvc<0) {
              log_fmt("Error receiving framebuffer");
              akpico_send_framebuffer();
              return;
            }
            log_fmt("Received framebuffer ok: %d",rcvc);
          } break;
          
        default: {
            memset(fb,0x04,fbw*fbh*2);
            return;
          }
      }
    }
    */
    
    akpico_send_framebuffer();
  }
}
