#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "job.h"
#include "queue.h"
#include "parser.h"
#include "utils.h"

#define MAX_JOBS 100
#define MLFQ_LEVELS 3
#define MLFQ_BOOST_INTERVAL 20

#define POLICY_SJF 1
#define POLICY_RR 2
#define POLICY_MLFQ 3

static void print_usage(const char* program) {
    printf("Usage: %s input_file policy [quantum]\n", program);
    printf("  policy = sjf | rr | mlfq\n");
    printf("  quantum is only used by rr (default 4)\n");
}

static int parse_policy(const char* policy) {
    if (strcmp(policy, "sjf") == 0) return POLICY_SJF;
    if (strcmp(policy, "rr") == 0) return POLICY_RR;
    if (strcmp(policy, "mlfq") == 0) return POLICY_MLFQ;
    return 0;
}

static void insert_ready_job(int policy, Queue* ready_queue, Queue mlfq[], Job* job) {
    job->state = READY;
    job->next = NULL;

    if (policy == POLICY_SJF) {
        enqueue_sjf(ready_queue, job);
    } else if (policy == POLICY_RR) {
        enqueue(ready_queue, job);
    } else {
        if (job->queue_level < 0) job->queue_level = 0;
        if (job->queue_level >= MLFQ_LEVELS) job->queue_level = MLFQ_LEVELS - 1;
        enqueue(&mlfq[job->queue_level], job);
    }
}

static Job* select_next_job(int policy, Queue* ready_queue, Queue mlfq[]) {
    int level;

    if (policy == POLICY_MLFQ) {
        for (level = 0; level < MLFQ_LEVELS; level++) {
            if (!is_empty(&mlfq[level])) {
                Job* job = dequeue(&mlfq[level]);
                job->state = RUNNING;
                job->time_slice_used = 0;
                return job;
            }
        }
        return NULL;
    }

    if (is_empty(ready_queue)) {
        return NULL;
    }

    Job* job = dequeue(ready_queue);
    job->state = RUNNING;
    if (policy == POLICY_RR) {
        job->time_slice_used = 0;
    }
    return job;
}

static void boost_all_jobs(Queue mlfq[], Job* current_job) {
    int level;
    int count;
    Job* job;

    for (level = 1; level < MLFQ_LEVELS; level++) {
        count = mlfq[level].size;
        while (count-- > 0) {
            job = dequeue(&mlfq[level]);
            job->queue_level = 0;
            enqueue(&mlfq[0], job);
        }
    }

    if (current_job != NULL && current_job->state == RUNNING) {
        current_job->queue_level = 0;
    }
}

static void update_waiting_stats(Queue* q, int is_ready_queue) {
    Job* temp = q->front;
    while (temp != NULL) {
        if (is_ready_queue) temp->ready_time++;
        else temp->io_time++;
        temp->total_time++;
        temp = temp->next;
    }
}

static void update_waiting_stats_mlfq(Queue mlfq[]) {
    int level;
    for (level = 0; level < MLFQ_LEVELS; level++) {
        update_waiting_stats(&mlfq[level], 1);
    }
}

static void print_ready_state(int policy, Queue* ready_queue, Queue mlfq[]) {
    if (policy == POLICY_MLFQ) {
        int i;
        for (i = 0; i < MLFQ_LEVELS; i++) {
            printf("Ready Queue L%d: ", i);
            print_queue(&mlfq[i]);
        }
    } else {
        printf("Ready Queue: ");
        print_queue(ready_queue);
    }
}

static void print_summary(Job jobs[], int total_jobs, int total_elapsed_time) {
    int i;
    int shortest = -1;
    int longest = -1;
    int sum_completion = 0;
    int sum_ready = 0;
    int sum_io = 0;

    printf("\nFinal Job Stats:\n");
    for (i = 0; i < total_jobs; i++) {
        int completion_time = jobs[i].finish_time - jobs[i].arrival_time;
        printf("PID=%d finish=%d turnaround=%d ready=%d io=%d running=%d\n",
               jobs[i].pid,
               jobs[i].finish_time,
               completion_time,
               jobs[i].ready_time,
               jobs[i].io_time,
               jobs[i].running_time);

        if (shortest == -1 || completion_time < shortest) shortest = completion_time;
        if (longest == -1 || completion_time > longest) longest = completion_time;
        sum_completion += completion_time;
        sum_ready += jobs[i].ready_time;
        sum_io += jobs[i].io_time;
    }

    printf("\nOverall Summary:\n");
    printf("Total jobs: %d\n", total_jobs);
    printf("Total elapsed time: %d\n", total_elapsed_time);
    printf("Shortest completion time: %d\n", shortest);
    printf("Longest completion time: %d\n", longest);
    printf("Average completion time: %.2f\n", (double)sum_completion / total_jobs);
    printf("Average ready time: %.2f\n", (double)sum_ready / total_jobs);
    printf("Average I/O time: %.2f\n", (double)sum_io / total_jobs);
}

int main(int argc, char* argv[]) {
    Job jobs[MAX_JOBS];
    Queue ready_queue;
    Queue io_queue;
    Queue mlfq[MLFQ_LEVELS];
    Job* current = NULL;
    int total_jobs;
    int policy;
    int rr_quantum = 4;
    const int mlfq_quantums[MLFQ_LEVELS] = {2, 4, 8};
    int clock = 0;
    int jobs_finished = 0;
    int i;

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    policy = parse_policy(argv[2]);
    if (policy == 0) {
        print_usage(argv[0]);
        return 1;
    }

    if (policy == POLICY_RR && argc >= 4) {
        rr_quantum = atoi(argv[3]);
        if (rr_quantum <= 0) {
            printf("RR quantum must be positive.\n");
            return 1;
        }
    }

    total_jobs = load_jobs(argv[1], jobs, MAX_JOBS);
    if (total_jobs <= 0) {
        printf("No jobs loaded.\n");
        return 1;
    }

    os_srand(1);
    init_queue(&ready_queue);
    init_queue(&io_queue);
    for (i = 0; i < MLFQ_LEVELS; i++) {
        init_queue(&mlfq[i]);
    }

    printf("Loaded %d jobs\n", total_jobs);
    printf("Policy: %s\n", argv[2]);
    if (policy == POLICY_RR) {
        printf("RR quantum: %d\n", rr_quantum);
    } else if (policy == POLICY_MLFQ) {
        printf("MLFQ quanta: 2, 4, 8; priority boost every %d ticks\n", MLFQ_BOOST_INTERVAL);
    }
    printf("\n--- Simulation Start ---\n");

    while (jobs_finished < total_jobs) {
        int io_count;
        int preempt_current = 0;

        printf("\nClock: %d\n", clock);

        if (policy == POLICY_MLFQ && clock > 0 && clock % MLFQ_BOOST_INTERVAL == 0) {
            boost_all_jobs(mlfq, current);
            printf("Priority boost applied\n");
        }

        for (i = 0; i < total_jobs; i++) {
            if (jobs[i].state == NEW && jobs[i].arrival_time == clock) {
                jobs[i].queue_level = 0;
                jobs[i].time_slice_used = 0;
                insert_ready_job(policy, &ready_queue, mlfq, &jobs[i]);
                printf("Job %d arrived and entered READY queue\n", jobs[i].pid);
            }
        }

        io_count = io_queue.size;
        for (i = 0; i < io_count; i++) {
            Job* job = dequeue(&io_queue);
            if (io_complete()) {
                if (policy == POLICY_MLFQ && job->queue_level < 0) {
                    job->queue_level = 0;
                }
                insert_ready_job(policy, &ready_queue, mlfq, job);
                printf("Job %d completed I/O and returned to READY queue\n", job->pid);
            } else {
                enqueue(&io_queue, job);
            }
        }

        if (policy == POLICY_SJF && current != NULL && current->state == RUNNING && !is_empty(&ready_queue)) {
            Job* best = peek_queue(&ready_queue);
            if (best->remaining_time < current->remaining_time ||
                (best->remaining_time == current->remaining_time && best->pid < current->pid)) {
                current->state = READY;
                insert_ready_job(policy, &ready_queue, mlfq, current);
                current = NULL;
                preempt_current = 1;
            }
        }

        if (policy == POLICY_MLFQ && current != NULL && current->state == RUNNING) {
            int higher_priority_ready = 0;
            for (i = 0; i < current->queue_level; i++) {
                if (!is_empty(&mlfq[i])) {
                    higher_priority_ready = 1;
                    break;
                }
            }
            if (higher_priority_ready) {
                current->state = READY;
                insert_ready_job(policy, &ready_queue, mlfq, current);
                current = NULL;
                preempt_current = 1;
            }
        }

        if (current == NULL) {
            current = select_next_job(policy, &ready_queue, mlfq);
            if (current != NULL) {
                printf("Running job %d\n", current->pid);
            }
        } else if (preempt_current) {
            current = select_next_job(policy, &ready_queue, mlfq);
            if (current != NULL) {
                printf("Running job %d\n", current->pid);
            }
        } else {
            printf("Continuing job %d\n", current->pid);
        }

        if (policy == POLICY_MLFQ) {
            update_waiting_stats_mlfq(mlfq);
        } else {
            update_waiting_stats(&ready_queue, 1);
        }
        update_waiting_stats(&io_queue, 0);

        if (current == NULL) {
            printf("CPU is idle\n");
            print_ready_state(policy, &ready_queue, mlfq);
            printf("IO Queue: ");
            print_queue(&io_queue);
            clock++;
            continue;
        }

        current->running_time++;
        current->total_time++;
        current->remaining_time--;
        current->time_slice_used++;

        if (current->remaining_time == 0) {
            current->state = FINISHED;
            current->finish_time = clock + 1;
            jobs_finished++;
            printf("Job %d finished at time %d\n", current->pid, current->finish_time);
            current = NULL;
        } else if (io_request()) {
            current->state = WAITING;
            current->time_slice_used = 0;
            enqueue(&io_queue, current);
            printf("Job %d requested I/O and moved to IO queue\n", current->pid);
            current = NULL;
        } else if (policy == POLICY_RR && current->time_slice_used >= rr_quantum) {
            current->state = READY;
            current->time_slice_used = 0;
            enqueue(&ready_queue, current);
            printf("Job %d time slice expired and returned to READY queue\n", current->pid);
            current = NULL;
        } else if (policy == POLICY_MLFQ && current->time_slice_used >= mlfq_quantums[current->queue_level]) {
            current->state = READY;
            current->time_slice_used = 0;
            if (current->queue_level < MLFQ_LEVELS - 1) {
                current->queue_level++;
            }
            insert_ready_job(policy, &ready_queue, mlfq, current);
            printf("Job %d used full quantum and moved to level %d\n", current->pid, current->queue_level);
            current = NULL;
        }

        print_ready_state(policy, &ready_queue, mlfq);
        printf("IO Queue: ");
        print_queue(&io_queue);

        clock++;
    }

    printf("\n--- Simulation End ---\n");
    print_summary(jobs, total_jobs, clock);
    return 0;
}