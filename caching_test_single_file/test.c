#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void main() {
    int* buffer;
    int rc, fd;
    float start, end, result;
    printf("%d\n", getpid());
    fd = open("./test.txt", O_RDONLY);
    // FILE *fp = fopen("./test.txt", "r");
    // if (fp < 0) {
    //     perror("failed opening file");
    //     return ;
    // }

    buffer = malloc(1024*128);
    // lseek(fd, 20480, SEEK_SET);
    start = clock();
    rc = read(fd, buffer, 1024*128);
    if (rc < 0) {
        perror("faild rading file");
        return;
    }
    printf("size: %dByte\n", rc);
    // rc = fread(buffer, 1, 1024*128, fp);
    //     if (rc < 0) {
    //     perror("failed reading file");
    //     return;
    // }
    end = clock();
    result = end - start;
    printf("128KB: Read success!\ntime: %lf\n", (double)result/CLOCKS_PER_SEC);
    free(buffer);
    close(fd);
}