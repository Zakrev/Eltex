#!/bin/bash

gcc -c add.c -fPIC
gcc -c sub.c -fPIC
gcc -c mul.c -fPIC
gcc -c div.c -fPIC
gcc -c complex.c -fPIC

gcc --shared -o add.so add.o
gcc --shared -o sub.so sub.o
gcc --shared -o mul.so mul.o
gcc --shared -o div.so div.o
gcc --shared -o complex.so complex.o

rm add.o sub.o mul.o div.o complex.o 2> /dev/null > /dev/null