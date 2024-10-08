/* tiny_hardware.c
 * Mostly copied from TinyCircuits and Arduino libs and stripped of C++.
 * I don't understand most of it.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void GluedSercom;

GluedSercom *sercom_getPERIPH_SPI1();
GluedSercom *sercom_getSercom3();

void sercom_disableSPI(GluedSercom *s);
void sercom_enableSPI(GluedSercom *s);
void sercom_enableWIRE(GluedSercom *s);
void sercom_initMasterWIRE(GluedSercom *s,uint32_t a);
void sercom_initSPI(GluedSercom *s,int a,int b,int c,int d);
void sercom_initSPIClock(GluedSercom *s,int a,uint32_t b);
uint8_t sercom_isAddressMatch(GluedSercom *s);
uint8_t sercom_isDataReadyWIRE(GluedSercom *s);
uint8_t sercom_isMasterReadOperationWIRE(GluedSercom *s);
uint8_t sercom_isRestartDetectedWIRE(GluedSercom *s);
uint8_t sercom_isSlaveWIRE(GluedSercom *s);
uint8_t sercom_isStopDetectedWIRE(GluedSercom *s);
void sercom_prepareAckBitWIRE(GluedSercom *s);
void sercom_prepareCommandBitsWire(GluedSercom *s,uint8_t a);
uint8_t sercom_sendDataMasterWIRE(GluedSercom *s,uint8_t a);
uint8_t sercom_sendDataSlaveWIRE(GluedSercom *s,uint8_t a);
void sercom_setBaudrateSPI(GluedSercom *s,uint8_t a);
void sercom_setClockModeSPI(GluedSercom *s,uint8_t a);
uint8_t sercom_startTransmissionWIRE(GluedSercom *s,uint8_t a,uint8_t b);
uint8_t sercom_transferDataSPI(GluedSercom *s,uint8_t a);

#ifdef __cplusplus
}
#endif

#include <sam.h>
#include <variant.h>
#include <wiring_constants.h>
#include <delay.h>

#define TS_USE_DELAY 1

#define TWI_CLOCK 100000

#define GPIO_ADDR 0x20

#define TSP_PIN_DC   22
#define TSP_PIN_CS   38
#define TSP_PIN_SHDN 27
#define TSP_PIN_RST  26
#define TSP_PIN_BT1  19
#define TSP_PIN_BT2  25
#define TSP_PIN_BT3  30
#define TSP_PIN_BT4  31

#define TS_SPI_SET_DATA_REG(x) {SERCOM4->SPI.DATA.bit.DATA=(x);}
#define TS_SPI_SEND_WAIT() {while(SERCOM4->SPI.INTFLAG.bit.DRE == 0);}

#define SPI_MIN_CLOCK_DIVIDER (uint8_t)(1 + ((F_CPU - 1) / 12000000))
#define SPI_MODE0 0x02
 
#define SPI_PAD_0_SCK_1 0
#define SPI_PAD_2_SCK_3 1
#define SPI_PAD_3_SCK_1 2
#define SPI_PAD_0_SCK_3 3

#define SERCOM_RX_PAD_0 0
#define SERCOM_RX_PAD_1 1
#define SERCOM_RX_PAD_2 2
#define SERCOM_RX_PAD_3 3

#define SERCOM_SPI_MODE_0 0
#define SERCOM_SPI_MODE_1 1
#define SERCOM_SPI_MODE_2 2
#define SERCOM_SPI_MODE_3 3

#define WIRE_WRITE_FLAG 0
#define WIRE_READ_FLAG 1

#define WIRE_MASTER_ACT_NO_ACTION 0
#define WIRE_MASTER_ACT_REPEAT_START 1
#define WIRE_MASTER_ACT_READ 2
#define WIRE_MASTER_ACT_STOP 3

/* Globals.
 */

// Wire
static GluedSercom *wiresc=0;
static uint8_t _uc_pinSDA=0;
static uint8_t _uc_pinSCL=0;
static uint8_t transmissionBegun=0;
static uint8_t txAddress=0;

// Adapted from Wire (don't bother figuring out RingBuffer<N>)
static uint8_t txBuffer[256];
static uint8_t txBufferp=0,txBufferc=0;

static GluedSercom *spisc=0;

static uint8_t tiny_terminate=0;

/* SPI.
 */
 
static void spi_begin() {
  spisc=sercom_getPERIPH_SPI1();
  
  // init,begin:
  pinPeripheral(PIN_SPI1_MISO, g_APinDescription[PIN_SPI1_MISO].ulPinType);
  pinPeripheral(PIN_SPI1_SCK, g_APinDescription[PIN_SPI1_SCK].ulPinType);
  pinPeripheral(PIN_SPI1_MOSI, g_APinDescription[PIN_SPI1_MOSI].ulPinType);
  
  // config:
  uint32_t clockFreq = (4000000 >= (F_CPU / SPI_MIN_CLOCK_DIVIDER) ? F_CPU / SPI_MIN_CLOCK_DIVIDER : 4000000);
  sercom_disableSPI(spisc);
  sercom_initSPI(spisc,PAD_SPI1_TX,PAD_SPI1_RX,0,0);
  sercom_initSPIClock(spisc,SERCOM_SPI_MODE_0,clockFreq);
  sercom_enableSPI(spisc);
}

static void spi_setClockDivider(uint8_t div) {
  if (div < SPI_MIN_CLOCK_DIVIDER) {
    sercom_setBaudrateSPI(spisc,SPI_MIN_CLOCK_DIVIDER);
  } else {
    sercom_setBaudrateSPI(spisc,div);
  }
}

static uint8_t spi_transfer(uint8_t v) {
  return sercom_transferDataSPI(spisc,v);
}

/* Wire.
 */
 
static void Wire_begin() {
  // ctor:
  wiresc=sercom_getSercom3();
  _uc_pinSDA=PIN_WIRE_SDA;
  _uc_pinSCL=PIN_WIRE_SCL;
  transmissionBegun = 0;
  // Wire.begin:
  sercom_initMasterWIRE(wiresc,TWI_CLOCK);
  sercom_enableWIRE(wiresc);
  pinPeripheral(_uc_pinSDA, g_APinDescription[_uc_pinSDA].ulPinType);
  pinPeripheral(_uc_pinSCL, g_APinDescription[_uc_pinSCL].ulPinType);
}

static void Wire_beginTransmission(uint8_t addr) {
  txAddress = addr;
  txBufferc=0;
  transmissionBegun = 1;
}

static uint8_t Wire_endTransmission() {
  transmissionBegun = 0;

  // Start I2C transmission
  if (!sercom_startTransmissionWIRE(wiresc,txAddress,WIRE_WRITE_FLAG)) {
    sercom_prepareCommandBitsWire(wiresc,WIRE_MASTER_ACT_STOP);
    return 2 ;  // Address error
  }

  // Send all buffer
  while (txBufferc>0) {
    // Trying to send data
    uint8_t v=txBuffer[txBufferp++];
    txBufferc--;
    if (!sercom_sendDataMasterWIRE(wiresc,v)) {
      sercom_prepareCommandBitsWire(wiresc,WIRE_MASTER_ACT_STOP);
      return 3 ;  // Nack or error
    }
  }

  return 0;
}

static uint8_t Wire_write(uint8_t v) {
  // No writing, without begun transmission or a full buffer
  if ( !transmissionBegun || (txBufferc==0xff) )
  {
    return 0 ;
  }

  txBuffer[txBufferp+txBufferc++]=v;

  return 1 ;
}

void SERCOM3_Handler() {
  if (sercom_isSlaveWIRE(wiresc)) {
    if (
      sercom_isStopDetectedWIRE(wiresc)||(
        sercom_isAddressMatch(wiresc)&&
        sercom_isRestartDetectedWIRE(wiresc)&&
        !sercom_isMasterReadOperationWIRE(wiresc)
      )
    ) {
      sercom_prepareAckBitWIRE(wiresc);
      sercom_prepareCommandBitsWire(wiresc,0x03);
    } else if (sercom_isAddressMatch(wiresc)) {
      sercom_prepareAckBitWIRE(wiresc);
      sercom_prepareCommandBitsWire(wiresc,0x03);
      if (sercom_isMasterReadOperationWIRE(wiresc)) {
        txBufferc=0;
        transmissionBegun=1;
      }
    } else if (sercom_isDataReadyWIRE(wiresc)) {
      if (sercom_isMasterReadOperationWIRE(wiresc)) {
        uint8_t c=0xff;
        if (txBufferc>0) {
          c=txBuffer[txBufferp++];
          txBufferc--;
        }
        transmissionBegun=sercom_sendDataSlaveWIRE(wiresc,c);
      }
    } else {
    }
  }
}

/* Private bits.
 */

static void writeGPIO(uint8_t regAddr, uint8_t regData) {
  Wire_beginTransmission(GPIO_ADDR);
  Wire_write(regAddr); 
  Wire_write(regData);
  Wire_endTransmission();
}

static void startCommand(void) {
  digitalWrite(TSP_PIN_DC,LOW);
  digitalWrite(TSP_PIN_CS,LOW);
}

static void startData(void) {
  digitalWrite(TSP_PIN_DC,HIGH);
  digitalWrite(TSP_PIN_CS,LOW);
}

static void endTransfer(void) {
  digitalWrite(TSP_PIN_CS,HIGH);
}

static void setBrightness(uint8_t brightness) {
  if(brightness>15)brightness=15;  
  startCommand();
  spi_transfer(0x87);//set master current
  spi_transfer(brightness);
  endTransfer();
}

static void on(void) {
  digitalWrite(TSP_PIN_SHDN,HIGH);
  startCommand();//if _externalIO, this will turn boost converter on
  delayMicroseconds(10000);
  spi_transfer(0xAF);//display on
  endTransfer();
}

static void off(void) {
  startCommand();
  spi_transfer(0xAE);//display off
  endTransfer();
  digitalWrite(TSP_PIN_SHDN,LOW);//SHDN
}

static void writeRemap(void) {
  uint8_t remap=(1<<5)|(1<<2);
  startCommand();
  spi_transfer(0xA0);//set remap
  spi_transfer(remap);
  endTransfer();
}

static void clearWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {  
  if(x>95||y>63)return;
  uint8_t x2=x+w-1;
  uint8_t y2=y+h-1;
  if(x2>95)x2=95;
  if(y2>63)y2=63;
  
  startCommand();
  spi_transfer(0x25);//clear window
  spi_transfer(x);
  spi_transfer(y);
  spi_transfer(x2);
  spi_transfer(y2);
  endTransfer();
#if TS_USE_DELAY
  delayMicroseconds(400);
#endif
}

/* Initialize.
 */
 
void tiny_init() {

  Wire_begin();
  usb_begin();

  spi_begin();
  sercom_setClockModeSPI(spisc,SERCOM_SPI_MODE_0);
  spi_setClockDivider(4);
  
  pinMode(TSP_PIN_SHDN,OUTPUT);
  pinMode(TSP_PIN_DC,OUTPUT);
  pinMode(TSP_PIN_CS,OUTPUT);
  pinMode(TSP_PIN_RST,OUTPUT);
  digitalWrite(TSP_PIN_SHDN,LOW);
  digitalWrite(TSP_PIN_DC,HIGH);
  digitalWrite(TSP_PIN_CS,HIGH);
  digitalWrite(TSP_PIN_RST,HIGH);
  pinMode(TSP_PIN_BT1,INPUT_PULLUP);
  pinMode(TSP_PIN_BT2,INPUT_PULLUP);
  pinMode(TSP_PIN_BT3,INPUT_PULLUP);
  pinMode(TSP_PIN_BT4,INPUT_PULLUP);
  //reset
  digitalWrite(TSP_PIN_RST,LOW);
  delay(5);
  digitalWrite(TSP_PIN_RST,HIGH);
  delay(10);
  
  const uint8_t init[32]={0xAE, 0xA1, 0x00, 0xA2, 0x00, 0xA4, 0xA8, 0x3F,
  0xAD, 0x8E, 0xB0, 0x0B, 0xB1, 0x31, 0xB3, 0xF0, 0x8A, 0x64, 0x8B,
  0x78, 0x8C, 0x64, 0xBB, 0x3A, 0xBE, 0x3E, 0x81, 0x91, 0x82, 0x50, 0x83, 0x7D};
  off();
  startCommand();
  for(uint8_t i=0;i<32;i++) spi_transfer(init[i]);
  endTransfer();
  //use libarary functions for remaining init
  setBrightness(10);
  writeRemap();
  clearWindow(0,0,96,64);
  on();
}

/* Update.
 */
 
int tiny_update() {
  uint8_t state=0;
  if (analogRead(25)<0x08) state|=1;
  if (analogRead(15)<0x08) state|=2;
  if (analogRead(42)<0x08) state|=4;
  if (analogRead(19)<0x08) state|=8;
  if (!digitalRead(44)) state|=16;
  if (!digitalRead(45)) state|=32;
  return state;
}

/* Framebuffer.
 */

void tiny_send_framebuffer(const void *fb/*96x64xbgr332*/) {
  startData();
  int i=96*64;
  const uint8_t *FB=fb;
  for (;i-->0;FB++) {
    TS_SPI_SET_DATA_REG(*FB);
    TS_SPI_SEND_WAIT();
  }
  endTransfer();
}
