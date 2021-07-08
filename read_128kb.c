#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void main() {
    char* buffer;
    int rc;
    float start, end, result;
    printf("%d\n", getpid());
    FILE *fp = fopen("./dummydir/testfile_128k.txt", "r");
    if (fp < 0) {
        perror("failed opening file");
        return ;
    }

    buffer = malloc(1024*128);

    start = clock();
    rc = fread(buffer, 1, 1024*128, fp);
    if (rc < 0) {
        perror("failed reading file");
        return;
    }
    end = clock();
    result = end - start;
    printf("128KB: Read success!\ntime: %lf\n", (double)result/CLOCKS_PER_SEC);
    free(buffer);
    fclose(fp);
}