#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
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
    char* dirpath;
    int rc, fd_single, size_single;
    int file_count = 0;
    int mode;
    struct dirent *ent;
    double o_start, o_end, r_start, r_end, m2_start, m2_end;
    DIR *dir;

    if(argc <= 2) {
        printf("input directory and mode(1 or 2)\nmeod 1: readahead\nmode 2: o_direct\n");
        return;
    }
    dirpath = argv[1];
    mode = ((int)argv[2][0])-48;
    strcat(dirpath, "/");
    dir = opendir(dirpath);
    if (dir == NULL) {
        perror("failed openning directory");
        return;
    }

    // while((ent = readdir(dir)) != NULL) {
    //     if(!strncmp(ent->d_name, ".", 1)) continue;
    //     file_count++;
    // }
    // int fd[file_count];
    // int size[file_count];
    // int size_max = 0;
    // double start[file_count];
    // double end[file_count];
    
    // int i = 0;
    
    // seekdir(dir, SEEK_SET);
    char* filepath = malloc(sizeof(char) * 255);
    // if(mode == 1) {
    //     o_start = get_time();
    //     while((ent = readdir(dir)) != NULL) {
    //         if(!strncmp(ent->d_name, ".", 1)) continue;
    //         strcat(filepath, argv[1]);
    //         strcat(filepath, ent->d_name);
    //         if((fd[i] = open(filepath, O_RDONLY)) < 0) {
    //             perror("failed open file");
    //             return;
    //         }
    //         size[i] = lseek(fd[i], 0, SEEK_END);
    //         if(size[i] > size_max) size_max = size[i];
    //         lseek(fd[i], 0, SEEK_SET);
    //         memset(filepath, 0, sizeof(char) * 255);
    //         i++;
    //     }
    //     o_end = get_time();

    //     r_start = get_time();
    //     buffer = malloc(size_max);
    //     for(int i=0; i<file_count; i++) {
    //         start[i] = get_time();
    //         if((rc = read(fd[i], buffer, size[i])) < 0 ) {
    //             perror("failed reading file");
    //             return;
    //         }
    //         end[i] = get_time();
    //         memset(buffer, 0, size_max);
    //     }
    //     r_end = get_time();
    //     printf("open running time: %lf\n", o_end - o_start);
    //     printf("read running time: %lf\n", r_end - r_start);
    // }
    // else {
        m2_start = get_time();
        buffer = (mode == 1) ? malloc(4096) : aligned_alloc(4096,4096);
        while((ent = readdir(dir)) != NULL) {
            if(!strncmp(ent->d_name, ".", 1)) continue;
            strcat(filepath, argv[1]);
            strcat(filepath, ent->d_name);
            if(mode == 1)
                fd_single = open(filepath, O_RDONLY);
            else
                fd_single = open(filepath, O_RDONLY, __O_DIRECT);
            size_single = lseek(fd_single, 0, SEEK_END);
            lseek(fd_single, 0, SEEK_SET);
            memset(filepath, 0, sizeof(char) * 255);
            if((rc = read(fd_single, buffer, size_single)) < 0 ) {
                perror("failed reading file");
                return;
            }
            memset(buffer, 0, 4096);
        }
        m2_end = get_time();
        printf("running time: %lf\n", m2_end - m2_start);
    // }
    free(buffer);
}