#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

double get_time() {
    double ms;
    int s;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    s = spec.tv_sec;
    ms = spec.tv_nsec / 10e8;
    return s + ms;
}

void main(int argc, char* argv[]) {
    char* buffer;
    char* filepath;
    int rc, fd, size, iter, pointer=0, prime=7;
    double start, end;
    if(argc == 1) {
        printf("input file path\n");
        return;
    }
    filepath = argv[1];

    start = get_time();
    if((fd = open(filepath, O_RDONLY | __O_DIRECT)) < 0) {
        perror("failed open file");
        exit(0);
    }
    
    size = lseek(fd, 0, SEEK_END);
    iter = size/4096;
    buffer = malloc(sizeof(char) * 4096);
    for(int i=0; i<iter; i++) {
        lseek(fd, pointer*4096, SEEK_SET);
        if((rc = read(fd, buffer, 4096)) < 0 ) {
            perror("failed reading file");
            return;
        }
        pointer += prime;
        if(pointer >= iter) pointer -= iter;
    }
    end = get_time();
    printf("running time: %lf\n", end - start);
}