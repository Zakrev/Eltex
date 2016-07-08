#!/bin/bash

gcc -c add.c
gcc -c sub.c
gcc -c mul.c
gcc -c div.c
gcc -c complex.c

ar rc libcalc.a add.o sub.o mul.o div.o complex.o

rm add.o sub.o mul.o div.o complex.o