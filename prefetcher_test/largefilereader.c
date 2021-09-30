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
    int mode;
    double start, end;
    if(argc <= 2) {
        printf("input file path and mode(1 ~ 2)\n");
        printf("mode 1: normal 4k random read\n");
        printf("mode 2: o_direct 4k random read\n");
        printf("mode 3: normal 4k sequantial read\n");
        printf("mode 4: o_direct 4k sequantial read\n");
        return;
    }
    filepath = argv[1];
    mode = ((int)argv[2][0])-48;

    
    if(mode == 1 || mode == 3) {
        fd = open(filepath, O_RDONLY);
        buffer = malloc(4096);
    }
    else if(mode == 2 || mode == 4) {
        fd = open(filepath, O_RDONLY | __O_DIRECT);
        buffer = aligned_alloc(4096, 4096);
    }

    if(fd < 0) {
        perror("failed open file");
        exit(0);
    }
    
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    start = get_time();
    if(mode == 1 || mode == 2) {
        iter = size/4096;
        for(int i=0; i<iter; i++) {
            lseek(fd, pointer*4096, SEEK_SET);
            if((rc = read(fd, buffer, 4096)) < 0 ) {
                perror("failed reading file");
                return;
            }
            pointer += prime;
            if(pointer >= iter) pointer -= iter;
            memset(buffer, 0, 4096);
        }
    }
    else if(mode == 3 || mode == 4) {
        while((rc = read(fd, buffer, 4096)) > 0) {
            memset(buffer, 0, 4096);
        }
    }
    end = get_time();
    printf("running time: %lf\n", end - start);
}