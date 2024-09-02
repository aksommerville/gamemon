#ifndef GAMEMON_INTERNAL_H
#define GAMEMON_INTERNAL_H

#include "gamemon.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/stat.h>

#define GAMEMON_RETRY_INTERVAL 5.0

struct gamemon {
  struct gamemon_delegate delegate;
  int fd;
  int fbw,fbh,pixfmt;
  int baud_rate;
  char *path;
  int input;
  double retry_time;
  void *tmp; // For converting framebuffers.
  int tmpa;
};

double gamemon_time_now();

int gamemon_fb_convert(
  void *dst,int dsta,int dstw,int dsth,int dstfmt,
  const void *src,int srcc,int srcw,int srch,int srcfmt
);

#endif
