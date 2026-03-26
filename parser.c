#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

static int compare_jobs_by_pid(const void* a, const void* b) {
    const Job* ja = (const Job*)a;
    const Job* jb = (const Job*)b;
    return ja->pid - jb->pid;
}

int load_jobs(const char* filename, Job jobs[], int max_jobs) {
    FILE* file = fopen(filename, "r");
    int count = 0;

    if (file == NULL) {
        printf("Error: cannot open file %s\n", filename);
        return -1;
    }

    while (count < max_jobs &&
           fscanf(file, "%d:%d:%d:%d",
                  &jobs[count].pid,
                  &jobs[count].arrival_time,
                  &jobs[count].service_time,
                  &jobs[count].priority) == 4) {

        jobs[count].remaining_time = jobs[count].service_time;
        jobs[count].state = NEW;

        jobs[count].ready_time = 0;
        jobs[count].io_time = 0;
        jobs[count].running_time = 0;
        jobs[count].total_time = 0;
        jobs[count].finish_time = -1;

        jobs[count].queue_level = 0;
        jobs[count].time_slice_used = 0;
        jobs[count].next = NULL;

        count++;
    }

    fclose(file);
    qsort(jobs, count, sizeof(Job), compare_jobs_by_pid);
    return count;
}