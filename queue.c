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

/* SJF: remaining_time 小的在前；若相同，PID 小的在前 */
void enqueue_sjf(Queue* q, Job* job) {
    job->next = NULL;

    if (q->front == NULL) {
        q->front = job;
        q->rear = job;
        q->size++;
        return;
    }

    Job* prev = NULL;
    Job* curr = q->front;

    while (curr != NULL) {
        if (job->remaining_time < curr->remaining_time) {
            break;
        }
        if (job->remaining_time == curr->remaining_time &&
            job->pid < curr->pid) {
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL) {
        job->next = q->front;
        q->front = job;
    } else {
        prev->next = job;
        job->next = curr;
    }

    if (curr == NULL) {
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

Job* peek_queue(Queue* q) {
    if (is_empty(q)) {
        return NULL;
    }
    return q->front;
}

void print_queue(Queue* q) {
    Job* curr = q->front;
    while (curr != NULL) {
        printf("[pid=%d rt=%d] ", curr->pid, curr->remaining_time);
        curr = curr->next;
    }
    printf("\n");
}