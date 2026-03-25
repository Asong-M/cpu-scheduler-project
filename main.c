#include <stdio.h>
#include <stdlib.h>
#include "job.h"
#include "queue.h"
#include "parser.h"
#include "utils.h"

#define MAX_JOBS 100

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s input_file\n", argv[0]);
        return 1;
    }

    Job jobs[MAX_JOBS];
    int total_jobs = load_jobs(argv[1], jobs, MAX_JOBS);

    if (total_jobs <= 0) {
        printf("No jobs loaded.\n");
        return 1;
    }

    os_srand(1);

    Queue ready_queue;
    Queue io_queue;
    init_queue(&ready_queue);
    init_queue(&io_queue);

    int clock = 0;
    int jobs_finished = 0;

    printf("Loaded %d jobs:\n", total_jobs);
    for (int i = 0; i < total_jobs; i++) {
        printf("PID=%d arrival=%d service=%d priority=%d\n",
               jobs[i].pid,
               jobs[i].arrival_time,
               jobs[i].service_time,
               jobs[i].priority);
    }

    printf("\n--- Simulation Start ---\n");

    while (jobs_finished < total_jobs) {
        printf("\nClock: %d\n", clock);

        // 1. add new arriving jobs
        for (int i = 0; i < total_jobs; i++) {
            if (jobs[i].arrival_time == clock) {
                jobs[i].state = READY;
                enqueue(&ready_queue, &jobs[i]);
                printf("Job %d arrived and entered READY queue\n", jobs[i].pid);
            }
        }

        // 2. check IO completion for jobs in io_queue
        int io_count = io_queue.size;
        for (int i = 0; i < io_count; i++) {
            Job* job = dequeue(&io_queue);
            if (io_complete()) {
                job->state = READY;
                enqueue(&ready_queue, job);
                printf("Job %d completed I/O and returned to READY queue\n", job->pid);
            } else {
                enqueue(&io_queue, job);
            }
        }

        // 3. run one job from ready queue
        if (!is_empty(&ready_queue)) {
            Job* current = dequeue(&ready_queue);
            current->state = RUNNING;

            printf("Running job %d\n", current->pid);

            current->remaining_time--;
            current->running_time++;

            if (current->remaining_time == 0) {
                current->state = FINISHED;
                current->finish_time = clock + 1;
                jobs_finished++;
                printf("Job %d finished at time %d\n", current->pid, current->finish_time);
            } else if (io_request()) {
                current->state = WAITING;
                enqueue(&io_queue, current);
                printf("Job %d requested I/O and moved to IO queue\n", current->pid);
            } else {
                current->state = READY;
                enqueue(&ready_queue, current);
            }
        } else {
            printf("CPU is idle\n");
        }

        // 4. update waiting stats
        Job* temp = ready_queue.front;
        while (temp != NULL) {
            temp->ready_time++;
            temp = temp->next;
        }

        temp = io_queue.front;
        while (temp != NULL) {
            temp->io_time++;
            temp = temp->next;
        }

        printf("Ready Queue: ");
        print_queue(&ready_queue);

        printf("IO Queue: ");
        print_queue(&io_queue);

        clock++;
    }

    printf("\n--- Simulation End ---\n");

    printf("\nFinal Job Stats:\n");
    for (int i = 0; i < total_jobs; i++) {
        printf("PID=%d finish=%d ready=%d io=%d running=%d\n",
               jobs[i].pid,
               jobs[i].finish_time,
               jobs[i].ready_time,
               jobs[i].io_time,
               jobs[i].running_time);
    }

    return 0;
}