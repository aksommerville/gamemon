#ifndef DEMO_INTERNAL_H
#define DEMO_INTERNAL_H

#include "lib/gamemon.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>

extern struct demo {
  volatile int sigc;
  int framec;
  int instate;
  struct gamemon *gamemon;
  uint8_t *fb;
  int fbw,fbh,fbfmt,fblen;
} demo;

#endif
