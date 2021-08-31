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
    int file_count = 0;
    struct dirent *ent;
    DIR *dir;

    if(argc == 1) {
        printf("input file size(4 ~ 128)\n");
        return;
    }
    dirpath = argv[1]; 
    strcat(dirpath, "/");
    size = atoi(dirpath);
    dir = opendir(dirpath);
    if (dir == NULL) {
        perror("failed openning directory");
        return;
    }

    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        file_count++;
    }
    FILE *fp[file_count];
    double start[file_count];
    double end[file_count];
    int i = 0;
    seekdir(dir, SEEK_SET);
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        char* filepath = malloc(sizeof(char) * 30);
        strcat(filepath, argv[1]);
        strcat(filepath, ent->d_name);
        if((fp[i] = fopen(filepath, "r")) < 0) {
            perror("failed open file");
            return;
        }
        free(filepath);
        i++;
    }

    buffer = malloc(1024*size);
    for(int i=0; i<file_count; i++) {
        start[i] = get_time();
        if((rc = fread(buffer, 1, 1024*size, fp[i])) < 0 ) {
            perror("failed reading file");
            return;
        }
        end[i] = get_time();
    }

    for(int i=0; i<file_count; i++) {
        printf("%lf,%lf\n", start[i], end[i]);
        fclose(fp[i]);
    }
    free(buffer);

    // buffer = malloc(1024*size);
    // start = get_time();
    // while ((ent = readdir(dir)) != NULL) {
    //     if(!strncmp(ent->d_name, ".", 1)) continue;
    //     char *filepath = malloc(sizeof(char) * 30);
    //     strcat(filepath, argv[1]);
    //     strcat(filepath, ent->d_name);
    //     fp = fopen(filepath, "r");
    //     if(fp < 0) {
    //         perror("failed reading file");
    //         return;
    //     }
    //     printf("%lf,", get_time());
    //     rc = fread(buffer, 1, 1024*size, fp);
    //     if (rc < 0) {
    //         perror("failed reading file");
    //         return;
    //     }
    //     printf("%lf\n", get_time());
    //     fclose(fp);
    //     free(filepath);
    // }
    // end = get_time();
    // printf("%lf\n", end-start);
}
