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


#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFF_SIZE (1024*(EVENT_SIZE + 16))

int* metadata;
int* metadata_size;
int num_of_thread;
int file_count = 0;
double g_start = 0, g_end = 0;

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
    int res;
    int* num = (int *)t;
    double start[file_count];
    double end[file_count];
    if(g_start == 0) g_start = get_time();
    for(int i=0; i<file_count; i++) {
        if((i+1) % num_of_thread != *num) continue;
        start[i] = get_time();
        if((res = posix_fadvise(metadata[i], 0, metadata_size[i], POSIX_FADV_WILLNEED)) != 0) {
            printf("fadvise error. error number: %d\n", res);
            return NULL;
        }
        end[i] = get_time();
    }
}

void main(int argc, char* argv[]) {
    int ifd = inotify_init();
    int wd;
    char buffer[BUFF_SIZE];
    char* dirpath;
    struct dirent *ent;
    DIR *dir;
    FILE *pfd;

    if(argc == 1) {
        printf("input options(-f, -t)\n");
        return;
    }

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
    // open directory
    if((dir = opendir(dirpath)) == NULL) {
        perror("failed open directory");
        return;
    }

    // get # of files
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        file_count++;
    }


    seekdir(dir, SEEK_SET);
    char* file_path = malloc(sizeof(char) * 255);
    int fd[file_count];
    int size[file_count];
    i = 0;

    //get metadata
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        strcat(file_path, dirpath);
        strcat(file_path, ent->d_name);
        if((fd[i] = open(file_path, O_RDONLY)) < 0) {
            printf("failed file open: %s, error number: %d\n", file_path, fd[i]);
            return NULL;
        }
        size[i] = lseek(fd[i], 0, SEEK_END);
        lseek(fd[i], 0, SEEK_SET);
        memset(file_path, 0, sizeof(char) * 255);
        i++;
    }
    metadata = fd;
    metadata_size = size;
    pthread_t p_thread[num_of_thread];
    
    // start monitoring directory
    wd = inotify_add_watch(ifd, dirpath, IN_ACCESS);
    if(wd == -1) {
        perror("Wrong directory name");
        return;
    }

    while(1) {
        int length, i = 0;
        length = read(ifd,buffer, BUFF_SIZE);
        if (i < length) {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            if(event->len) {
                if(event->mask & IN_ACCESS) {
                    // monitor ACCESS
                    int args[num_of_thread];
                    for(int j=0; j<num_of_thread; j++) {
                        args[j] = j;
                        int thr;
                        if(thr = pthread_create(&p_thread[j], NULL, prefetche, (void *)&args[j])<0) {
                            perror("thread create error:");
                            exit(0);
                        }
                    }
                    
                    for(int j=0; j<num_of_thread; j++) {
                        pthread_join(p_thread[j], NULL);
                    }
                    g_end = get_time();
                    printf("%lf\n", g_end - g_start);
                    break;
                }
            }
            i += EVENT_SIZE + event->len;
        }

    }
}
