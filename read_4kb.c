#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

void main() {
    char* buffer;
    int rc;
    float start, end, result;
    struct dirent *ent;
    DIR *dir = opendir("./4kbs");
    printf("%d\n", getpid());
    if (dir == NULL) {
        perror("failed openning directory");
        return;
    }
    FILE *fp;

    buffer = malloc(1024*4);
    start = clock();
    while ((ent = readdir(dir)) != NULL) {
        char filepath[100] = "./4kbs/";
        strcat(filepath, ent->d_name);
        // printf("%s\n", filepath);
        fp = fopen(filepath, "r");
        if(fp < 0) {
            perror("failed reading file");
            return;
        }
        rc = fread(buffer, 1, 1024*4, fp);
        if (rc < 0) {
            perror("failed reading file");
            return;
        }
    }
    end = clock();
    result = end - start;
    printf("4KB x 32: Read success!\ntime: %lf\n", (double)result/CLOCKS_PER_SEC);
}