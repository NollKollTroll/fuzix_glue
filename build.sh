#!/bin/bash

echo \#### converting fuzix binary
cp ~/FUZIX/Images/pz1/fuzix.rom ./fuzix_kernel
xxd -i fuzix_kernel fuzix_kernel.h
#echo \#### build and upload main code to Teensy
#arduino --upload fuzix_glue.ino
echo \#### Done!
