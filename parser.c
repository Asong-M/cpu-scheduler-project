#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int load_jobs(const char* filename, Job jobs[], int max_jobs) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: cannot open file %s\n", filename);
        return -1;
    }

    int count = 0;

    while (count < max_jobs &&
           fscanf(file, "%d:%d:%d:%d",
                  &jobs[count].pid,
                  &jobs[count].arrival_time,
                  &jobs[count].service_time,
                  &jobs[count].priority) == 4) {

        jobs[count].remaining_time = jobs[count].service_time;
        jobs[count].state = READY;

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
    return count;
}