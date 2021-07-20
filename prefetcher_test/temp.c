#include <stdio.h>
#include <stdlib.h>

void main() {
    char* a = "128kb";
    int b;
    b = atoi(a);
    printf("%d\n", b);
    // if(&a[0] == "h") printf("%c\n", a[0]);
}