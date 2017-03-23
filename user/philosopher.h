#ifndef __PHILOSOPHER_H
#define __PHILOSOPHER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include "PL011.h"

#include "libc.h"

typedef struct {
  int clean;
  int write;
  int block;
  int read;
  int pid;
}pipe

#endif
