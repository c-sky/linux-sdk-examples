#!/bin/sh

if [[ "$#" != 1 ]] ; then
    echo "please enter file name!"
    exit 1
fi

NAME="$1"

./csky_ffmpeg_media_player_example -c mpeg4 -d /dev/fb0 -i $NAME -m /dev/video3
