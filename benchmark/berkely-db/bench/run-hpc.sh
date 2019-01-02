#!/bin/bash
set -x

make outputclean
sleep 1
./ex_thread -i 300000 -r $((THREADS/2)) -w $((THREADS/2)) 

