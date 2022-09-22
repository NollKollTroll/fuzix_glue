#!/bin/bash

echo \#### converting fuzix binary
cp ../FUZIX/Kernel/platform-pz1/fuzix.img ./fuzix_kernel
xxd -i fuzix_kernel fuzix_kernel.h
#echo \#### build and upload main code to Teensy
#arduino --upload fuzix_glue.ino
echo \#### Done!
