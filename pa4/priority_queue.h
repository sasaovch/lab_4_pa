#pragma once
#ifndef __PQUEUE_H
#define __PQUEUE_H

#include <stdio.h>
#include <stdlib.h>

#include "ipc.h"

#define MAX 100

typedef struct {
    local_id pipe_id;
    timestamp_t timestamp;
} PriorityQueueElement;

typedef struct {
    int items[MAX][2];
    int size;
} PriorityQueue;

extern PriorityQueue pqueue;

void sort(int index);
void push(PriorityQueueElement value);

PriorityQueueElement pop();
PriorityQueueElement peek();
void printQueue();

#endif
