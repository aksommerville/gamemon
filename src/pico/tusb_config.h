/* tusb_config.h
 * tinyusb I guess creates this as part of its build process, which I still haven't figured out.
 */

#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#define CFG_TUSB_MCU OPT_MCU_RP2040

// Not sure what this is (beyond "Root Hub Port"), but it controls TUSB_OPT_DEVICE_ENABLED, nothing compiles without it.
//#define TUD_OPT_RHPORT 0
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE|OPT_MODE_HIGH_SPEED)

#define CFG_TUD_CDC 1
#define CFG_TUD_CDC_TX_BUFSIZE 256
#define CFG_TUD_CDC_RX_BUFSIZE 4096

#endif
