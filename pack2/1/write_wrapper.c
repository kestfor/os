#include <sys/syscall.h>
#include <unistd.h>

int print(int fd, char *buf, size_t len) {
    return syscall(1, fd, buf, len);
}

int main() {
    print(STDOUT_FILENO, "hello world", 11);
}