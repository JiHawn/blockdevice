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

char* dirpath;
char** file_list;
int num_of_thread;
int file_count = 0;

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
    int fd[file_count];
    int res;
    off_t size[file_count];
    int* num = (int *)t;
    char* targetfile = malloc(sizeof(char) * 255);
    double start[file_count];
    double end[file_count];
    double t_start;
    double t_end;
    for(int i=0; i<file_count; i++) {
        fd[i] = -1;
    }
    for(int i=0; i<file_count; i++) {
        if((i+1) % num_of_thread != *num) continue;
        strcat(targetfile, dirpath);
        strcat(targetfile, file_list[i]);
        printf("%s\n", targetfile);
        if((fd[i] = open(targetfile, O_RDONLY)) < 0) {
            printf("failed file open: %s, error number: %d\n", targetfile, fd[i]);
            return NULL;
        }
        size[i] = lseek(fd[i], 0, SEEK_END);
        lseek(fd[i], 0, SEEK_SET);
        memset(targetfile, 0, sizeof(char) * 255);
    }

    t_start = get_time();
    for(int i=0; i<file_count; i++) {
        if(fd[i] > 0) {
            start[i] = get_time();
            if((res = posix_fadvise(fd[i], 0, size[i], POSIX_FADV_WILLNEED)) != 0) {
                printf("fadvise error. error number: %d\n", res);
                return NULL;
            }
            end[i] = get_time();
        }
    }
    t_end = get_time();

    for(int i=0; i<file_count; i++) {
        if(fd[i] > 0) {
            // printf("%d,%lf,%lf\n", i, start[i], end[i]);
            close(fd[i]);
        }
    }
    printf("%lf\n", t_end - t_start);
}

void main(int argc, char* argv[]) {
    int fd, ifd = inotify_init();
    int wd, rc ,size;
    double start, end;
    char buffer[BUFF_SIZE];
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
                    // end = get_time();
                    // printf("%lf\n", end-start);
                    break;
                }
            }
            i += EVENT_SIZE + event->len;
        }

    }
}
