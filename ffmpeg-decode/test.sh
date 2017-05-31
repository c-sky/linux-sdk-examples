#!/bin/sh
# For C-SKY eragon - 2017-05-28

export LD_LIBRARY_PATH=/mnt/nfs_lzj/lib
./v4l2_decode -c mpeg4 -d /dev/fb0 -i ./s5_800x480.mp4 -m /dev/video3
