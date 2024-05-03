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
            sleep(30);
        } else {
            printf("zombie parent pid: %d\n", getpid());
            printf("child pid: %d", new_pid);
            exit(2);
        }
    } else {
        sleep(50);
    }

}
