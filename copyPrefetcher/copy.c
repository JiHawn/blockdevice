#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

#define FILE_MAX 256
#define FILE_SIZE_MAX 5000

int extern errno;
int copyLock = 0;
int prefetcheLock = 0;
int ioLimit = INT_MAX;

struct src_dest {
    char* src;
    char* dest;
};

void* copy(void* p) {
    DIR *srcDir, *destDir;
    int srcFd, destFd;
    char *srcFilepath = malloc(sizeof(char) * FILE_MAX);
    char *destFilepath = malloc(sizeof(char) * FILE_MAX);
    char *srcFilename, *destFilename;
    char *buffer = malloc(FILE_SIZE_MAX);
    struct src_dest *sd = (struct src_dest *)p;
    char *src = sd->src;
    char *dest = sd->dest;
    struct dirent *ent;
    int res;
    int srcLength, destLength;
    int size;
    int ioNum;
    if((res = mkdir(dest, 0777)) == -1) {
        if(errno != 17) {
            perror("make dir");
            exit(1);
        }
    }
    if((srcDir = opendir(src)) == NULL) {
        perror("open src directory");
        exit(1);
    }
    strcat(srcFilepath, src);
    strcat(srcFilepath, "/");
    srcLength = strlen(src) + 1;
    srcFilename = srcFilepath + srcLength;

    strcat(destFilepath, dest);
    strcat(destFilepath, "/");
    destLength = strlen(dest) + 1;
    destFilename = destFilepath + destLength;

    while((ent = readdir(srcDir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        while(copyLock) {;}
        strcpy(srcFilename, ent->d_name);
        strcpy(destFilename, ent->d_name);
        if((srcFd = open(srcFilepath, O_RDONLY)) < 0) {
            perror("src file open");
            exit(1);
        }
        if((destFd = open(destFilepath, O_RDWR | O_CREAT| O_TRUNC, 0666)) < 0) {
            perror("dest file open");
            exit(1);
        }
        size = lseek(srcFd, 0, SEEK_END);
        lseek(srcFd, 0, SEEK_SET);
        if((res = read(srcFd, buffer, size)) < 0) {
            perror("src file read");
            exit(1);
        }
        if((res = write(destFd, buffer, size)) < 0) {
            perror("dest file write");
            exit(1);
        }
        close(srcFd);
        close(destFd);
        ioNum++;
        if(ioNum == ioLimit) {
            copyLock = 1;
            prefetcheLock = 0;
            ioNum = 0;
        }
    }
    prefetcheLock = 0;
}

void* prefetche(void *p) {
    DIR *dir;
    struct dirent *ent;
    char *filepath = malloc(sizeof(char) * FILE_MAX);
    char *filename;
    int size;
    int fd;
    int res;
    int ioNum = 0;
    struct src_dest *sd = (struct src_dest *)p;
    char* path = sd->src;

    if((dir = opendir(path)) == NULL) {
        perror("failed open directory");
    }

    strcat(filepath, path);
    strcat(filepath, "/");
    filename = filepath + strlen(path) + 1;
    
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        while(prefetcheLock) {;}
        strcpy(filename, ent->d_name);
        if((fd = open(filepath, O_RDONLY)) < 0) {
            perror("file open");
            exit(1);
        }
        size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        if((res = posix_fadvise(fd, 0, size, POSIX_FADV_WILLNEED)) != 0) {
            perror("fadvise");
            exit(1);
        }
        close(fd);
        ioNum++;
        if(ioNum == ioLimit) {
            prefetcheLock = 1;
            copyLock = 0;
            ioNum = 0;
        }
    }
    copyLock = 0;
}

int main(int argc, char* argv[]) {
    struct src_dest *args;
    pthread_t cpThread;
    pthread_t prThread;
    int thr;

    if(argc < 3) {
        printf("input error: %s [src] [dest] [ioNum] <- option\n", argv[0]);
        exit(1);
    }
    args->src = argv[1];
    args->dest = argv[2];
    if(argc == 4) ioLimit = atoi(argv[3]);

    if((thr = pthread_create(&prThread, NULL, prefetche, (void *)args)) <0) {
        perror("prefetcher thread create error");
        exit(1);
    }

    if((thr = pthread_create(&cpThread, NULL, copy, (void *)args)) <0) {
        perror("copy thread create error");
        exit(1);
    }

    pthread_join(prThread, NULL);
    pthread_join(cpThread, NULL);
}