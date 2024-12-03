#pragma once
#ifndef  __PIPES_CONST_H
#define  __PIPES_CONST_H

#include "ipc.h"
#include <stdio.h>

typedef struct {
  int pm[10][10][2];
  local_id fork_id;
  timestamp_t local_time;
  int N;
  int is_mutexl;
  int received_done_msg;
} Info;

extern FILE *elf;
extern FILE *plf;

extern Info pipe_info;

extern int pm[10][10][2];

#endif
