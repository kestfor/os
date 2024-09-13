#bin/bash

gcc -fPIC -c thread.c
gcc -shared -o libthread.so thread.o
gcc -c main.c
gcc main.o -lthread -L. -o main
rm main.o
rm thread.o
