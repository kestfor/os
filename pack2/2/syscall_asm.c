int print(int fd, char *buf, int len) {
    long res;
    long syscallNum = 1;
    asm ("movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "syscall\n\t"
         "movq %%rax, %0"
            : "=r" (res)
            : "r" (syscallNum), "r" ((long) fd), "r" (buf), "r" ((long) len)
            : "rax", "rdi", "rsi", "rdx");
    return res;
}

int main() {
    print(1, "hello world", 11);
}