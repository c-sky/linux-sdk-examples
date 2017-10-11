#!/bin/sh

if [[ "$#" != 1 ]] ; then
    echo "please enter file name!"
    exit 1
fi

NAME="$1"
./csky_fb_example -d /dev/fb0 -p yuv420 -f $NAME --hdmi

