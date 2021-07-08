#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void main() {
    int* buffer;
    int ra, fd, size;
    float start, end, result;
    printf("%d\n", getpid());
    fd = open("./test.txt", O_RDONLY);
    if(fd < 0) {
        perror("failed reading file");
        return;
    }
    size = lseek(fd, 0, SEEK_END);
    start = clock();
    // ra = posix_fadvise(fd, 0, size, POSIX_FADV_WILLNEED);
    ra = readahead(fd, 0, size);
    if (ra < 0) {
        perror("caching failed");
        return;
    }
    end = clock();
    result = end - start;
    printf("caching success!\ntime: %lf\n", (double)result/CLOCKS_PER_SEC);
    close(fd);
}