#include "gamemon_internal.h"

/* Delete.
 */
 
void gamemon_del(struct gamemon *gamemon) {
  if (!gamemon) return;
  if (gamemon->path) free(gamemon->path);
  if (gamemon->fd>=0) close(gamemon->fd);
  if (gamemon->tmp) free(gamemon->tmp);
  free(gamemon);
}

/* New.
 */

struct gamemon *gamemon_new(
  const struct gamemon_delegate *delegate,
  const char *path,
  int baud_rate
) {
  if (!path||!path[0]) return 0;
  struct gamemon *gamemon=calloc(1,sizeof(struct gamemon));
  if (!gamemon) return 0;
  gamemon->fd=-1;
  if (!(gamemon->path=strdup(path))) {
    gamemon_del(gamemon);
    return 0;
  }
  gamemon->baud_rate=baud_rate;
  if (delegate) memcpy(&gamemon->delegate,delegate,sizeof(struct gamemon_delegate));
  return gamemon;
}

/* Trivial accessors.
 */

int gamemon_get_input(const struct gamemon *gamemon) {
  if (!gamemon) return 0;
  return gamemon->input;
}

int gamemon_get_fd(const struct gamemon *gamemon) {
  if (!gamemon) return -1;
  return gamemon->fd;
}

int gamemon_is_connected(const struct gamemon *gamemon) {
  if (!gamemon) return 0;
  return (gamemon->fd>=0)?1:0;
}

int gamemon_get_fb_format(int *w,int *h,const struct gamemon *gamemon) {
  if (!gamemon) return 0;
  if (w) *w=gamemon->fbw;
  if (h) *h=gamemon->fbh;
  return gamemon->pixfmt;
}

/* Explicitly drop connection.
 * Do not call back.
 */

void gamemon_disconnect(struct gamemon *gamemon) {
  if (!gamemon) return;
  if (gamemon->fd<0) return;
  close(gamemon->fd);
  gamemon->fd=-1;
  gamemon->fbw=gamemon->fbh=gamemon->pixfmt=0;
  gamemon->input=0;
}
