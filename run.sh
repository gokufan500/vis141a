#!/bin/bash
mpiexec -np 1 -pernode -x DISPLAY=:0.0 ./program
