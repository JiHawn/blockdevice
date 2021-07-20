#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

void main(int argc, char* argv[]) {
    char* buffer;
    char dirpath[30];
    char* filepath;
    int rc, size;
    float start, end, result;
    struct dirent *ent;
    DIR *dir;
    FILE *fp;

    if(argc == 1) {
        printf("input file size(4kb ~ 128kb)\n");
        return;
    }
    strcat(argv[1], "/");
    size = atoi(argv[1]);
    dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("failed openning directory");
        return;
    }

    buffer = malloc(1024*size);
    start = clock();
    while ((ent = readdir(dir)) != NULL) {
        if(!strncmp(ent->d_name, ".", 1)) continue;
        char *filepath = malloc(sizeof(char) * 30);
        strcat(filepath, argv[1]);
        strcat(filepath, ent->d_name);
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
        fclose(fp);
        free(filepath);
        // printf("file name: %s\n", filepath);
        // sleep(1);
    }   
    end = clock();
    result = end - start;
    printf("%lf\n", (double)result/CLOCKS_PER_SEC);
}