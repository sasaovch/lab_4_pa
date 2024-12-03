#include "priority_queue.h"

PriorityQueue pqueue;

void sort(int index) {  
    if (index
        && (
                pqueue.items[(index - 1)][1] < pqueue.items[index][1]
                || (
                    pqueue.items[(index - 1)][1] == pqueue.items[index][1]
                    && pqueue.items[(index - 1)][0] < pqueue.items[index][0]
                )
            )
        ) {
            PriorityQueueElement temp;
            temp.pipe_id = pqueue.items[(index - 1)][0];
            temp.timestamp = pqueue.items[(index - 1)][1];

            pqueue.items[(index - 1)][0] = pqueue.items[index][0];
            pqueue.items[(index - 1)][1] = pqueue.items[index][1];

            pqueue.items[index][0] = temp.pipe_id;
            pqueue.items[index][1] = temp.timestamp;
   
            sort((index - 1));
    }
}

void printQueue() {
    if (pqueue.size == 0) {
        printf("Priority Queue is empty.\n");
        return;
    }
    
    printf("Priority Queue (size %d):\n", pqueue.size);
    for (int i = 0; i < pqueue.size; i++) {
        printf("Index %d: %d - %d\n", i, pqueue.items[i][0], pqueue.items[i][1]);
    }
}

void push(PriorityQueueElement value) {
    pqueue.items[pqueue.size][0] = value.pipe_id;
    pqueue.items[pqueue.size++][1] = value.timestamp;

    sort(pqueue.size - 1);
}

PriorityQueueElement pop() {
    PriorityQueueElement item;
    item.pipe_id = pqueue.items[--pqueue.size][0];
    item.timestamp = pqueue.items[pqueue.size][1];

    return item;
}

PriorityQueueElement peek() {
    PriorityQueueElement item;
    item.pipe_id = pqueue.items[pqueue.size - 1][0];
    item.timestamp = pqueue.items[pqueue.size - 1][1];
    
    return item;
}
