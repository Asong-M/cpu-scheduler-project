#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void init_queue(Queue* q) {
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

int is_empty(Queue* q) {
    return q->front == NULL;
}

void enqueue(Queue* q, Job* job) {
    job->next = NULL;

    if (q->rear == NULL) {
        q->front = job;
        q->rear = job;
    } else {
        q->rear->next = job;
        q->rear = job;
    }

    q->size++;
}

Job* dequeue(Queue* q) {
    if (is_empty(q)) {
        return NULL;
    }

    Job* temp = q->front;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    temp->next = NULL;
    q->size--;
    return temp;
}

void print_queue(Queue* q) {
    Job* curr = q->front;
    while (curr != NULL) {
        printf("[pid=%d rt=%d] ", curr->pid, curr->remaining_time);
        curr = curr->next;
    }
    printf("\n");
}