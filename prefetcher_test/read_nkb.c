#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

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
    char* dirpath;
    char* filepath;
    int rc, size;
    int num_of_thread;
    double start, end, result;
    struct dirent *ent;
    DIR *dir;
    FILE *fp;

    if(argc == 1) {
        printf("input file size(4 ~ 128)\n");
        return;
    }
    dirpath = argv[1];
    strcat(dirpath, "kb/");
    size = atoi(dirpath);
    dir = opendir(dirpath);
    if (dir == NULL) {
        perror("failed openning directory");
        return;
    }

    buffer = malloc(1024*size);
    start = get_time();
    while ((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        char *filepath = malloc(sizeof(char) * 30);
        strcat(filepath, argv[1]);
        strcat(filepath, ent->d_name);
        fp = fopen(filepath, "r");
        if(fp < 0) {
            perror("failed reading file");
            return;
        }
        printf("%lf,", get_time());
        rc = fread(buffer, 1, 1024*size, fp);
        if (rc < 0) {
            perror("failed reading file");
            return;
        }
        printf("%lf\n", get_time());
        fclose(fp);
        free(filepath);
    }
    end = get_time();
    // printf("%lf\n", end-start);
}
