#define _GNU_SOURCE
#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFF_SIZE (1024*(EVENT_SIZE + 16))
#define CACHED 1
#define NOT_CACHED 0

void main() {
    int fd, ifd = inotify_init();
    int wd;
    int size;
    int flag = NOT_CACHED;
    char buffer[BUFF_SIZE];
    char target[20] = "4kbs";
    struct dirent *ent;
    DIR *dir;

    printf("PID: %d\n", getpid());

    wd = inotify_add_watch(ifd, target, IN_ACCESS);

    while(1) {
        int length, i = 0;
        length = read(ifd,buffer, BUFF_SIZE);
        if (i < length) {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            // printf ("[debug] wd=%d mask=%d cookie=%d len=%d dir=%s\n", event->wd, event->mask, event->cookie, event->len, (event->mask & IN_ISDIR)?"yes":"no");
            if(event->len) {
                if(event->mask & IN_ACCESS) {
                    // printf("\"%s\" was accessed!\n", event->name);
                    if(!flag) {
                        dir = opendir(target);
                        if(dir == NULL) {
                            perror("failed openning directroy");
                            inotify_rm_watch(ifd, wd);
                            return;
                        }
                        while((ent = readdir(dir)) != NULL) {
                            char targetfile[30] = "4kbs/";
                            strcat(targetfile, ent->d_name);
                            fd = open(targetfile, O_RDONLY);
                            if(fd < 0) {
                                perror("failed caching file");
                                inotify_rm_watch(ifd, wd);
                                return;
                            }
                            size = lseek(fd, 0, SEEK_END);
                            if(size <= 4096) {
                                lseek(fd, 0, SEEK_SET);
                                readahead(fd, 0, size);
                            }
                            close(fd);
                        }
                        flag = CACHED;
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
}