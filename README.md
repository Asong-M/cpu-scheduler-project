# CPU Scheduler Project

A simple CPU scheduling simulator written in C. Supports three scheduling policies: SJF, Round Robin, and MLFQ.

## How to Build

```bash
gcc -o scheduler main.c parser.c queue.c utils.c
```

## How to Run

```bash
./scheduler input.txt sjf
./scheduler input.txt rr [quantum]
./scheduler input.txt mlfq
```

- `quantum` is optional for RR, defaults to 4 if not specified.

## Input File Format

Each line describes one job:

```
pid:arrival_time:service_time:priority
```

Example (`input.txt`):
```
123:0:10:1
124:1:20:0
125:3:5:2
```

## Scheduling Policies

- **SJF** — Shortest Job First, preemptive. Jobs with less remaining time run first.
- **RR** — Round Robin. Each job gets a fixed time quantum before being cycled out.
- **MLFQ** — Multi-Level Feedback Queue. 3 levels with quanta 2, 4, 8. Priority boost every 20 ticks.

## Notes on Determinism

I/O events are simulated randomly but deterministically. The RNG is seeded with `1` before the main loop. For each tick, `io_complete()` is called for all waiting jobs (FIFO) before `io_request()` is called for the running job. This ensures everyone's output matches.

## Sample Output (SJF, input.txt)

```
         | Total time      | Total time       | Total time   |
  Job#   | in ready to run | in sleeping on   | in system    |
         | state           | I/O state        |              |
=========+==================+==================+==============+
123      | 1               | 0                | 11           |
124      | 14              | 0                | 34           |
125      | 0               | 8                | 13           |
=========+==================+==================+==============+
Total simulation run time: 35
Total number of jobs: 3
Shortest job completion time: 11
Longest job completion time: 34
Average job completion time: 19.33
Average time in ready queue: 5.00
Average time sleeping on I/O state: 2.67
```
