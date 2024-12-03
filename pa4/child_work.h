#pragma once
#ifndef  __CHILD_WORK_H
#define  __CHILD_WORK_H

#include "ipc.h"

typedef struct {
    local_id fork_id;
    timestamp_t child_time;
    int N;
    int is_mutexl;
} ChildState;

int init_child_work(void* __child_state);
int handle_transfers(void* __child_state);

#endif
