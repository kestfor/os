#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int global_var;

int main() {
    int local_var = 10;
    global_var = 9;
    printf("local var: addr: %p, value: %d\n", &local_var, local_var);
    printf("global var: addr: %p, value: %d\n", &global_var, global_var);
    printf("pid: %d\n", getpid());
    int pid = fork();

    if (pid == -1) {
        perror("fork fail");
        return 1;
    }

    if (pid == 0) {
        int new_pid = fork();

        if (new_pid == -1) {
            perror("fork fail");
            return 1;
        }

        if (new_pid == 0) {
            printf("parent pid of child before: %d\n", getppid());
            fflush(stdout);
            sleep(5);
            printf("parent pid of child after: %d\n", getppid());
            fflush(stdout);
            sleep(30);
        } else {
            sleep(2);
            printf("zombie parent pid: %d\n", getpid());
            printf("child pid: %d\n", new_pid);
            exit(5);
        }
    } else {
        sleep(50);
    }

}
