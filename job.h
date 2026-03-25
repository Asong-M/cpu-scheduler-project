#ifndef JOB_H
#define JOB_H

typedef enum {
    READY,
    RUNNING,
    WAITING,
    FINISHED
} JobState;

typedef struct Job {
    int pid;
    int arrival_time;
    int service_time;
    int remaining_time;
    int priority;

    JobState state;

    int ready_time;
    int io_time;
    int running_time;
    int total_time;

    int finish_time;

    int queue_level;      // for MLFQ later
    int time_slice_used;  // for RR / MLFQ later

    struct Job* next;
} Job;

#endif