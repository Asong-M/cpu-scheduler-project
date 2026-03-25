#ifndef UTILS_H
#define UTILS_H

void os_srand(unsigned int seed);
int os_rand(void);

int io_request(void);
int io_complete(void);

#endif