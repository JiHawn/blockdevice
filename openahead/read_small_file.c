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
    int rc, fd, size;
    struct dirent *ent;
    double m2_start, m2_end;
    DIR *dir;

    if(argc < 2) {
        printf("input directory\n");
        return;
    }
    dirpath = argv[1];
    strcat(dirpath, "/");
    dir = opendir(dirpath);
    if (dir == NULL) {
        perror("failed openning directory");
        return;
    }
    
    int i = 0;
    
    seekdir(dir, SEEK_SET);
    char* filepath = malloc(sizeof(char) * 255);
    buffer = malloc(1024*16);
    m2_start = get_time();
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        strcat(filepath, argv[1]);
        strcat(filepath, ent->d_name);
        fd = openahead(filepath);
        size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        memset(filepath, 0, sizeof(char) * 255);
        if((rc = read(fd, buffer, size)) < 0 ) {
            perror("failed reading file");
            return;
        }
        memset(buffer, 0, 1024*16);
    }
    m2_end = get_time();
    printf("running time: %lf\n", m2_end - m2_start);
    free(buffer);
}
