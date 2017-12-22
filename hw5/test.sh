#!/bin/bash -x
./parser $1
aarch64-linux-gnu-gcc -g -O0 -static main.S
qemu-aarch64 a.out
