#ifndef NANO_CLOCK_H
#define NANO_CLOCK_H

#include <time.h>

#define INIT_NANO_CLOCK struct timespec clock

// delay in nanoseconds
#define NSEC_DELAY(time)                                                                                      \
    clock_gettime(CLOCK_MONOTONIC, &clock);                                                                   \
    clock.tv_nsec += time;                                                                                    \
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &clock, NULL)                                             \

// delay in seconds
#define SEC_DELAY(time)                                                                                       \
    clock_gettime(CLOCK_MONOTONIC, &clock);                                                                   \
    clock.tv_sec += time;                                                                                     \
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &clock, NULL)                                             \

#endif // NANO_CLOCK_H