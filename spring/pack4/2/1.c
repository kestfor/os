#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("pid: %d\n", getpid());
    sleep(3);
    execv(argv[0], argv);
    printf("hello world");
}