#ifndef QUEUE_H
#define QUEUE_H

#include "job.h"

typedef struct {
    Job* front;
    Job* rear;
    int size;
} Queue;

void init_queue(Queue* q);
int is_empty(Queue* q);
void enqueue(Queue* q, Job* job);
Job* dequeue(Queue* q);
void print_queue(Queue* q);

#endif