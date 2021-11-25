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
int *copyLock;
int *prefetcheLock;
int ioLimit = INT_MAX;
int numOfPair;
int numOfFile = 0;

struct src_dest {
    char* src;
    char* dest;
    int num;
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
    int num = sd->num;
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

    int i = 0;
    while((ent = readdir(srcDir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        if((i+1) % numOfPair != num) {
            i++;
            continue;
        }
        while(copyLock[num]) {;}
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
            copyLock[num] = 1;
            prefetcheLock[num] = 0;
            ioNum = 0;
        }
        i++;
    }
    prefetcheLock[num] = 0;
}

void* prefetche(void *p) {
    DIR *dir;
    struct dirent *ent;
    char *filepath = malloc(sizeof(char) * FILE_MAX);
    char *filename;
    int size[numOfFile/numOfPair+1];
    int fd[numOfFile/numOfPair+1];
    int res;
    int ioNum = 0;
    struct src_dest *sd = (struct src_dest *)p;
    char* path = sd->src;
    int num = sd->num;

    if((dir = opendir(path)) == NULL) {
        perror("failed open directory");
    }

    for(int i=0; i<numOfFile/numOfPair+1; i++) {
        size[i] = -1;
        fd[i] = -1;
    }

    strcat(filepath, path);
    strcat(filepath, "/");
    filename = filepath + strlen(path) + 1;

    int i=0;
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".",1)) continue;
        if((i+1) % numOfPair != num) {
            i++;
            continue;
        }
        strcpy(filename, ent->d_name);
        if((fd[i] = open(filepath, O_RDONLY)) < 0) {
            perror("file open in prefetcher");
            exit(1);
        }
        size[i] = lseek(fd[i], 0, SEEK_END);
        lseek(fd[i], 0, SEEK_SET);
        i++;
    }
    
    for(int i=0; i<numOfFile/numOfPair+1; i++) {
        if(fd[i] < 0) continue;
        while(prefetcheLock[num]) {;}
        if((res = posix_fadvise(fd[i], 0, size[i], POSIX_FADV_WILLNEED)) != 0) {
            perror("fadvise");
            exit(1);
        }
        ioNum++;
        if(ioNum == ioLimit) {
            prefetcheLock[num] = 1;
            copyLock[num] = 0;
            ioNum = 0;
        }
    }

    for(int i=0; i<numOfFile/numOfPair+1; i++) {
        if(fd[i] <0) continue;
        close(fd[i]);
    }
    copyLock[num] = 0;
}

int main(int argc, char* argv[]) {
    if(argc < 4) {
        printf("input error: %s [src] [dest] [numOfPair] [ioNum] <- option\n", argv[0]);
        exit(1);
    }
    numOfPair = atoi(argv[3]);
    if(argc == 5) ioLimit = atoi(argv[4]);

    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(argv[1])) == NULL) {
        perror("failed open directory in main");
        exit(1);
    }
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        numOfFile++;
    }
    closedir(dir);

    int cpLock[numOfPair];
    int prLock[numOfPair];
    for(int i=0; i<numOfPair; i++) {
        cpLock[i] = 0;
        prLock[i] = 0;
    }
    copyLock = cpLock;
    prefetcheLock = prLock;
    struct src_dest args[numOfPair];
    pthread_t cpThread[numOfPair];
    pthread_t prThread[numOfPair];
    int thr;

    for(int i=0; i<numOfPair; i++) {
        args[i].src = argv[1];
        args[i].dest = argv[2];
        args[i].num = i;
        
        if((thr = pthread_create(&prThread[i], NULL, prefetche, (void *)&args[i])) <0) {
            perror("prefetcher thread create error");
            exit(1);
        }

        if((thr = pthread_create(&cpThread[i], NULL, copy, (void *)&args[i])) <0) {
            perror("copy thread create error");
            exit(1);
        }
    }
    
    for(int i=0; i<numOfPair; i++) {
        pthread_join(prThread[i], NULL);
        pthread_join(cpThread[i], NULL);
    }
}