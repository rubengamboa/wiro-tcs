#!/bin/bash

gcc -I/usr/src/linux-`uname -r`/include/ -O -D__KERNEL__ -fno-common -fomit-frame-pointer -mpreferred-stack-boundary=2 -pipe -march=i586 -DMODULE -c ik220.c

# EnDat 2.1 Ausleseprogramm
cc IK220Con.c ik220_lib.c   -o ik220con

# Drehgeberprogramm -> Verwendung der Ausleseroutine READ48
cc IK220_read48.c ik220_lib.c -o ik220_read48