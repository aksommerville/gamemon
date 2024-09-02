/* akpico.h
 * Stripped-down interface to PicoSystem, with C linkage.
 */
 
#ifndef AKPICO_H
#define AKPICO_H

#include <stdint.h>

void akpico_init();
void akpico_update();
uint8_t akpico_get_buttons();
void akpico_send_framebuffer();
void *akpico_get_framebuffer(int *w,int *h,int *pixelsize); // All constant.

#endif
