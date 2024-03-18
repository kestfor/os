#include <unistd.h>


int main() {
    write(STDOUT_FILENO, "hello world", 11);
}