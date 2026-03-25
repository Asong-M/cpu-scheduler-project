#include <stdlib.h>
#include "utils.h"

#define CHANCE_OF_IO_REQUEST 10
#define CHANCE_OF_IO_COMPLETE 4

void os_srand(unsigned int seed) {
    srand(seed);
}

int os_rand(void) {
    return rand();
}

int io_request(void) {
    return (os_rand() % CHANCE_OF_IO_REQUEST == 0);
}

int io_complete(void) {
    return (os_rand() % CHANCE_OF_IO_COMPLETE == 0);
}