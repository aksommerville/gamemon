/* gamemon.h
 * Host-side API for monitoring gameplay on some device attached via USB-Serial.
 * You can create one gamemon context for the game's run lifetime.
 * We manage connection and disconnection of the device intelligently.
 */
 
#ifndef GAMEMON_H
#define GAMEMON_H

struct gamemon;

#define GAMEMON_INPUT_LEFT    0x01
#define GAMEMON_INPUT_RIGHT   0x02
#define GAMEMON_INPUT_UP      0x04
#define GAMEMON_INPUT_DOWN    0x08
#define GAMEMON_INPUT_A       0x10
#define GAMEMON_INPUT_B       0x20
#define GAMEMON_INPUT_C       0x40
#define GAMEMON_INPUT_D       0x80

#define GAMEMON_PIXFMT_Y1BE          0x00
#define GAMEMON_PIXFMT_Y2BE          0x10
#define GAMEMON_PIXFMT_Y4BE          0x20
#define GAMEMON_PIXFMT_CGA4BE        0x21
#define GAMEMON_PIXFMT_Y8            0x30
#define GAMEMON_PIXFMT_BGR332        0x31 /* TinyArcade */
#define GAMEMON_PIXFMT_Y16BE         0x40
#define GAMEMON_PIXFMT_XRGB4444BE    0x41 /* PicoSystem */
#define GAMEMON_PIXFMT_BGR565BE      0x42 /* TinyArcade, but our client doesn't use this one. */
#define GAMEMON_PIXFMT_Y32BE         0x50
#define GAMEMON_PIXFMT_RGBX          0x51
#define GAMEMON_PIXFMT_BGRX          0x52
#define GAMEMON_PIXFMT_XRGB          0x53
#define GAMEMON_PIXFMT_XBGR          0x54
#define GAMEMON_PIXFMT_Y24BE         0xf0
#define GAMEMON_PIXFMT_RGB           0xf1
#define GAMEMON_PIXFMT_BGR           0xf2

#define GAMEMON_FRAMEBUFFER_SIZE_LIMIT 1024

/* All delegate callbacks are optional.
 * (input) gets triggered only on real changes. Not necessarily every Input packet from the device.
 * (disconnected) does not trigger on deletion.
 */
struct gamemon_delegate {
  void *userdata;
  void (*connected)(void *userdata);
  void (*disconnected)(void *userdata);
  void (*fb_format)(int w,int h,int pixfmt,void *userdata);
  void (*input)(int state,int pvstate,void *userdata);
};

void gamemon_del(struct gamemon *gamemon);

/* Create a context.
 * We don't perform any I/O during this call, that's all deferred to gamemon_update().
 * If (baud_rate) zero, we'll assume we can open the device and write it to directly.
 * Nonzero, we'll run `stty` first. For TinyArcade, it should be 9600.
 * It's fair to not provide a delegate at all. Everything you need, you can poll us for it.
 */
struct gamemon *gamemon_new(
  const struct gamemon_delegate *delegate,
  const char *path,
  int baud_rate
);

/* Call often.
 * We may block at times when I/O is happening.
 * Delegate callbacks can only happen during this call, and gamemon_send_framebuffer().
 */
int gamemon_update(struct gamemon *gamemon);

/* Returns bitfields, GAMEMON_INPUT_*.
 */
int gamemon_get_input(const struct gamemon *gamemon);

int gamemon_get_fd(const struct gamemon *gamemon);

/* Drop the current connection.
 * Does not trigger delegate's disconnect callback.
 * We'll try to reestablish it at the next update.
 */
void gamemon_disconnect(struct gamemon *gamemon);

/* It's possible to be connected but still unable to send framebuffers.
 */
int gamemon_is_connected(const struct gamemon *gamemon);

/* Return the 8-bit pixel format, and populate (w,h), if it's been established.
 * Zeroes if we're disconnected, or format not established yet.
 */
int gamemon_get_fb_format(int *w,int *h,const struct gamemon *gamemon);

/* Convert to the device's format if necessary, and send out.
 * If we're not connected or format not established, returns zero without doing anything.
 * Returns >0 if we did send.
 * <0 only for malformed argument or I/O error.
 * (c) is for validation purposes and we're strict about it.
 * It is an error to provide more data than needed (and less, obviously).
 */
int gamemon_send_framebuffer(
  struct gamemon *gamemon,
  int w,int h,int pixfmt,
  const void *v,int c
);

/* Size in bits for a given format ID.
 * Zero if anything's fishy.
 */
int gamemon_pixel_size(int pixfmt);

/* Size in bytes of a framebuffer with the given format, and minimum stride.
 * Everything that goes over the wire must have minimum stride.
 */
int gamemon_measure_framebuffer(int w,int h,int pixfmt);

#endif
