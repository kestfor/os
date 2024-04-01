#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/ptrace.h>
#include "syscallent.h"

void parent(int child_pid) {
    int status;
    waitpid(child_pid, &status, 0);
    ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD);
    struct ptrace_syscall_info info;

    while (!WIFEXITED(status)) {
        ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
        waitpid(child_pid, &status, 0);

        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) {
            ptrace(PTRACE_GET_SYSCALL_INFO, child_pid, sizeof(info), &info);
            if (info.entry.nr >= table_size) {
                printf("syscall №%llu (", info.entry.nr);
                printf("%llu, %llu, %llu, %llu, %llu, %llu)", info.entry.args[0],
                       info.entry.args[1], info.entry.args[2], info.entry.args[3], info.entry.args[4], info.entry.args[5]);
            } else {
                printf("%s(", table[info.entry.nr].name);
                for (int i = 0; i < table[info.entry.nr].args_num - 1; i++) {
                    printf("%llu, ", info.entry.args[i]);
                }
                printf("%llu)", info.entry.args[table[info.entry.nr].args_num - 1]);
            }
            ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
            waitpid(child_pid, &status, 0);


            ptrace(PTRACE_GET_SYSCALL_INFO, child_pid, sizeof(info), &info);
            printf(" = %lld\n", info.exit.rval);
        }
    }

}


void child(int argc, char *argv[], char *env[]) {
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    execve(argv[1], argv + 1, env);
}

int main(int argc, char *argv[], char *env[]) {
    if (argc == 1) {
        return 0;
    }

    int pid = fork();
    if (pid < 0) {
        perror("fork fail");
        exit(1);
    }

    if (pid == 0) {
        child(argc, argv, env);
    } else {
        parent(pid);
    }
}