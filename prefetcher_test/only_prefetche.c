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
double app_start;

double get_time() {
    double ms;
    int s;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    s = spec.tv_sec;
    ms = spec.tv_nsec / 10e8;
    return s + ms;
}

void* prefetche(void* t) {
    int count = file_count/2 + 1;
    int fd[count];
    int fadvise_res;
    off_t size[count];
    int *num = (int *)t;
    char* targetfile = malloc(sizeof(char) * 255);
    double start[count];
    double end[count];
    for(int i=0; i<count; i++) {
        fd[i] = -1;
    }
    int j = 0;
    for(int i=0; i<file_count; i++) {
        if((i+1) % num_of_thread != *num) continue;
        strcat(targetfile, dirpath);
        strcat(targetfile, file_list[i]);
        if((fd[j] = open(targetfile, O_RDONLY)) < 0) {
            perror("failed file open");
            return NULL;
        }
        size[j] = lseek(fd[j], 0, SEEK_END);
        lseek(fd[j], 0, SEEK_SET);
        memset(targetfile, 0, sizeof(char) * 255);
        j++;
    }
    
    while(1) {
        if(app_start == get_time() - 2) break;
    }

    for(int i=0; i<count; i++) {
        if(fd[i] < 0) break;
        start[i] = get_time();
        if((fadvise_res = posix_fadvise(fd[i], 0, size[i], POSIX_FADV_WILLNEED)) != 0) {
                printf("fadvise error. error number: %d\n", fadvise_res);
                return NULL;
            }
        end[i] = get_time();
    }

    for(int i=0; i<count; i++) {
        if(fd[i] < 0) break;
        printf("%d,%lf,%lf\n", *num+(num_of_thread*i), start[i], end[i]);
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
    seekdir(dir, SEEK_SET);
    char* files[file_count];
    i = 0;
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        files[i] = ent->d_name;
        i++;
    }
    file_list = files;
    
    pthread_t p_thread[num_of_thread];
    int args[num_of_thread];
    app_start = get_time();
    for(int i=0; i<num_of_thread; i++) {
        args[i] = i;
        int thr;
        if((thr = pthread_create(&p_thread[i], NULL, prefetche, (void *)&args[i])) < 0) {
            perror("thread create error:");
            exit(0);
        }
    }

    for(int i=0; i<num_of_thread; i++) {
        pthread_join(p_thread[i], NULL);
    }
}
