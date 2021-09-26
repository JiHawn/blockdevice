#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

double get_time() {
    double ms;
    int s;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    s = spec.tv_sec;
    ms = spec.tv_nsec / 10e8;
    return s + ms;
}

int openahead(char* filepath) {
    int fd;
    int fad;
    if((fd = open(filepath, O_RDONLY)) < 0) {
        perror("open failed");
        exit(1);
    }
    if((fad = posix_fadvise(fd, 0, 4096, POSIX_FADV_WILLNEED)) < 0) {
        perror("caching failed");
        exit(1);
    }
    return fd;
}

void main(int argc, char* argv[]) {
    char* buffer;
    char* dirpath;
    int rc, mode;
    int file_count = 0;
    struct dirent *ent;
    double o_start, o_end, r_start, r_end;
    DIR *dir;

    if(argc <= 2) {
        printf("input directory and using openahead\n1: normal open\n2: openahead\n");
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
    
    int i = 0;

    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        file_count++;
    }
    int fd[file_count];
    int size[file_count];
    int size_max = 0;
    double start[file_count];
    double end[file_count];
    
    seekdir(dir, SEEK_SET);
    char* filepath = malloc(sizeof(char) * 255);
    buffer = malloc(1024*16);
    o_start = get_time();
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        strcat(filepath, argv[1]);
        strcat(filepath, ent->d_name);
        if(mode == 1) {
            if((fd[i] = open(filepath, O_RDONLY)) < 0) {
                perror("failed open file");
                return;
            }
        } else {
            fd[i] = openahead(filepath);
        }
        size[i] = lseek(fd[i], 0, SEEK_END);
        if(size[i] > size_max) size_max = size[i];
        lseek(fd[i], 0, SEEK_SET);
        memset(filepath, 0, sizeof(char) * 255);
        i++;
    }
    o_end = get_time();

    r_start = get_time();
    buffer = malloc(size_max);
    for(int i=0; i<file_count; i++) {
        start[i] = get_time();
        if((rc = read(fd[i], buffer, size[i])) < 0 ) {
            perror("failed reading file");
            return;
        }
        end[i] = get_time();
        memset(buffer, 0, size_max);
    }
    r_end = get_time();
    printf("open running time: %lf\n", o_end - o_start);
    printf("read running time: %lf\n", r_end - r_start);
    free(buffer);
}
