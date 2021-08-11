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
int num_of_thread;

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
    struct dirent *ent;
    int fd;
    off_t size;
    int* num = (int *)t;
    DIR* dir = opendir(dirpath);
    char* targetfile = malloc(sizeof(char) * 255);
    if(dir == NULL) {
        perror("failed openning directroy");
        exit(-1);
    }
    int i = 1;
    while((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) {
            i++;
            continue;
        }
        if(i % num_of_thread != *num) {
            i++;
            continue;
        }
        strcat(targetfile, dirpath);
        strcat(targetfile, ent->d_name);
        fd = open(targetfile, O_RDONLY);
        if(fd < 0) {
            perror("failed caching file");
            exit(-1);
        }
        size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        printf("%lf\n", get_time());
        posix_fadvise(fd, 0, size, POSIX_FADV_WILLNEED);
        // printf("Thread%d: %s\n", *num, targetfile);
        
        close(fd);
        memset(targetfile, 0, sizeof(char) * 255);
        i++;
    }
    free(targetfile);
}

void main(int argc, char* argv[]) {
    int fd, ifd = inotify_init();
    int wd, rc ,size;
    int flag = 0;
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
                        int thr = pthread_create(&p_thread[j], NULL, prefetche, (void *)&args[j]);
                        if(thr<0) {
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
