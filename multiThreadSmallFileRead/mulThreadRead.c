#define _GNU_SOURCE
#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

char* dirpath;
char** file_list;
int num_of_thread;
int file_count = 0;
int app_start;

double get_time() {
    double ms;
    int s;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    s = spec.tv_sec;
    ms = spec.tv_nsec / 10e8;
    return s + ms;
}

void* multiRead(void* t) {
    int count = file_count/num_of_thread + 1;
    int fd[count];
    int rd;
    off_t size[count];
    int *num = (int *)t;
    char* targetfile = malloc(sizeof(char) * 255);
    DIR *dir;
    struct dirent *ent;
    double start[count];
    double end[count];
    for(int i=0; i<count; i++) {
        fd[i] = -1;
    }

    if((dir = opendir(dirpath)) == NULL) {
        perror("failed open directory");
        return NULL;
    }

    int i = 0;
    int j = 0;
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        if((i+1) % num_of_thread != *num) {
            i++;
            continue;
        }
        strcat(targetfile, dirpath);
        strcat(targetfile, ent->d_name);
        if((fd[j] = open(targetfile, O_RDONLY)) < 0) {
            perror("failed file open");
            return NULL;
        }
        size[j] = lseek(fd[j], 0, SEEK_END);
        lseek(fd[j], 0, SEEK_SET);
        memset(targetfile, 0, sizeof(char) * 255);
        i++;
        j++;
    }

    for(int i=0; i<count; i++) {
        if(fd[i] < 0) break;
        char *buffer = malloc(size[i]);
        if((rd = read(fd[i], buffer, size[i])) < 0) {
            perror("multiRead read error");
            return NULL;
        }
    }
}

void* prefetche(void* t) {
    int count = file_count/num_of_thread + 1;
    int fd[count];
    int fadvise_res;
    off_t size[count];
    int *num = (int *)t;
    char* targetfile = malloc(sizeof(char) * 255);
    DIR *dir;
    struct dirent *ent;
    double start[count];
    double end[count];
    for(int i=0; i<count; i++) {
        fd[i] = -1;
    }

    if((dir = opendir(dirpath)) == NULL) {
        perror("failed open directory");
        return NULL;
    }

    int i = 0;
    int j = 0;
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        if((i+1) % num_of_thread != *num) {
            i++;
            continue;
        }
        strcat(targetfile, dirpath);
        strcat(targetfile, ent->d_name);
        if((fd[j] = open(targetfile, O_RDONLY)) < 0) {
            perror("failed file open");
            return NULL;
        }
        size[j] = lseek(fd[j], 0, SEEK_END);
        lseek(fd[j], 0, SEEK_SET);
        memset(targetfile, 0, sizeof(char) * 255);
        i++;
        j++;
    }

    for(int i=0; i<count; i++) {
        if(fd[i] < 0) break;
        if((fadvise_res = posix_fadvise(fd[i], 0, size[i], POSIX_FADV_WILLNEED)) != 0) {
            perror("fadvise error");
            return NULL;
        }
    }

    for(int i=0; i<count; i++) {
        if(fd[i] < 0) break;
        close(fd[i]);
    }
}

void main(int argc, char* argv[]) {
    DIR* dir;
    struct dirent* ent;
    int i = 1;
    while(argv[i]) {
        if(argv[i][0] == '-') {
            if(argv[i][1] == 'f') {
                dirpath = argv[i+1];
            }
            else if(argv[i][1] == 't') {
                num_of_thread = atoi(argv[i+1]);
            }
            else {
                printf("option error\n");
                return;
            }
        }
        else {
            printf("input options (-f, -t)\n");
        }
        i += 2;
    }

    strcat(dirpath, "/");
    if((dir = opendir(dirpath)) == NULL) {
        perror("failed open directory");
        return;
    }
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        file_count++;
    }
    closedir(dir);

    pthread_t p_thread[num_of_thread];
    pthread_t r_thread[num_of_thread];
    int args[num_of_thread];
    app_start = get_time();
    for(int i=0; i<num_of_thread; i++) {
        args[i] = i;
        int thr;
        if((thr = pthread_create(&p_thread[i], NULL, prefetche, (void *)&args[i])) < 0) {
            perror("thread create error:");
            exit(0);
        }
        if((thr = pthread_create(&r_thread[i], NULL, multiRead, (void *)&args[i])) < 0) {
            perror("thread create error:");
            exit(0);
        }
    }

    for(int i=0; i<num_of_thread; i++) {
        pthread_join(p_thread[i], NULL);
    }
    for(int i=0; i<num_of_thread; i++) {
        pthread_join(r_thread[i], NULL);
    }
}
