#!/bin/bash

cp xupnb.bin a.bin
gcc upsim.c -o comp_sim
./comp_sim \< @1
cp sim2 a.bin
