#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

void main() {
    char* buffer;
    int fd, rc, status;
    off_t size;
    float start, end, result;
    struct dirent *ent;
    FILE *fp;
    pid_t pid;
    
    DIR *dir = opendir("./4kbs");
    if (dir == NULL) {
        perror("failed openning directory");
        return;
    }
    start = clock();
    pid = fork();
    if (pid == 0) {
        buffer = malloc(1024*4);
        while ((ent = readdir(dir)) != NULL) {
            char filepath[100] = "./4kbs/";
            strcat(filepath, ent->d_name);
            // printf("%s\n", filepath);
            fd = open(filepath, O_RDONLY);
            if(fd < 0) {
                perror("failed reading file");
                return;
            }
            size = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            readahead(fd, 0, size);
        }
        free(buffer);
        close(fd);
        closedir(dir);
        return;
    }
    else {
        buffer = malloc(1024*4);
        while ((ent = readdir(dir)) != NULL) {
            char filepath[100] = "./4kbs/";
            strcat(filepath, ent->d_name);
            // printf("%s\n", filepath);
            fp = fopen(filepath, "r");
            if(fp < 0) {
                perror("failed reading file");
                return;
            }
            rc = fread(buffer, 1, 1024*4, fp);
            if (rc < 0) {
                perror("failed reading file");
                return;
            }
        }
        wait(&status);
    }
    end = clock();
    result = end - start;
    printf("4KB x 32: caching and read success!\ntime: %lf\n", (double)result/CLOCKS_PER_SEC);
    free(buffer);
    fclose(fp);
    closedir(dir);
}