#define _GNU_SOURCE
#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>


#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFF_SIZE (1024*(EVENT_SIZE + 16))

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
    int fd, ifd = inotify_init();
    int wd, rc ,size;
    int flag = 0;
    double start, end;
    char buffer[BUFF_SIZE];
    char target[20];
    struct dirent *ent;
    DIR *dir;
    FILE *pfd;

    if(argc == 1) {
        printf("input directory name!\n");
        return;
    }
    strcat(argv[1], "kb/");
    strcat(target, argv[1]);
    // printf("PID: %d\n", getpid());

    wd = inotify_add_watch(ifd, target, IN_ACCESS);
    if(wd == -1) {
        perror("Worng directory name");
        return;
    }

    while(1) {
        int length, i = 0;
        length = read(ifd,buffer, BUFF_SIZE);
        if (i < length) {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            if(event->len) {
                if(event->mask & IN_ACCESS) {
                    dir = opendir(target);
                    if(dir == NULL) {
                        perror("failed openning directroy");
                        inotify_rm_watch(ifd, wd);
                        return;
                    }
                    while((ent = readdir(dir)) != NULL) {
                            if(!strncmp(ent->d_name, ".", 1)) continue;
                            char *targetfile = malloc(sizeof(char) * 30);
                            strcat(targetfile, target);
                            strcat(targetfile, ent->d_name);
                            fd = open(targetfile, O_RDONLY);
                            if(fd < 0) {
                                perror("failed caching file");
                                inotify_rm_watch(ifd, wd);
                                return;
                            }
                            size = lseek(fd, 0, SEEK_END);
                            lseek(fd, 0, SEEK_SET);
                            // if(!flag) {
                            //     start = get_time();
                            //     flag = 1;
                            // } 
                            // printf("%lf\n", get_time());
                            posix_fadvise(fd, 0, size, POSIX_FADV_WILLNEED);
                            
                            close(fd);
                            free(targetfile);
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