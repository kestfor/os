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
        int parent_pid = getppid();
        int own_pid = getpid();

        printf("from child: pid: %d, parent pid: %d\n", own_pid, parent_pid);
        printf("from child: local var: addr: %p, value: %d\n", &local_var, local_var);
        printf("from child: global var: addr: %p, value: %d\n", &global_var, global_var);

        global_var = -1;
        local_var = -2;

        printf("from child: local var: addr: %p, value: %d\n", &local_var, local_var);
        printf("from child: global var: addr: %p, value: %d\n", &global_var, global_var);

        exit(2);

    } else {
        printf("from parent: local var: addr: %p, value: %d\n", &local_var, local_var);
        printf("from parent: global var: addr: %p, value: %d\n", &global_var, global_var);
        sleep(30);
        int status;
        waitpid(pid, &status, 0);
        printf("exit status: %d\n", WEXITSTATUS(status));
    }

}