#!/bin/sh
# For C-SKY eragon - 2017-05-27

if [[ "$#" != 1 ]] ; then
    echo "please enter file name!"
    exit 1
fi

NAME="$1"
./csky_v4l2_decode_example -c mpeg4 -d /dev/fb0 -i $NAME -m /dev/video3
