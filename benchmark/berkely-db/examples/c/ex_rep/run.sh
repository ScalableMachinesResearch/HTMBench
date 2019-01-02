#!/bin/bash
killall -9 ex_reg_mgr
rm -rf data
mkdir data
./ex_reg_mgr -M -h data -l 127.0.0.1:6000 -r 127.0.0.1:6001 -n 2 &
./ex_reg_mgr -C -h data -l 127.0.0.1:6001 -r 127.0.0.1:6000 -n 2 &
