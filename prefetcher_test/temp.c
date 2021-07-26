#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void main() {
    double ms;
    int s;
    double ret;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    s = spec.tv_sec;
    ms = spec.tv_nsec / 10e8;
    ret = s + ms;
    // if (ms > 99) {
    //     s++;
    //     ms = 0;
    // }

    printf("Current time: %lf seconds since the Epoch\n",s+ms);
}