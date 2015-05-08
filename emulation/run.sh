#!/bin/bash

cp compiler.bin a.bin
gcc serial.c -o compiler
./compiler \< @1
cp sim2 a.bin
rm -f compiler
gcc network.c -o network -lm
./network
