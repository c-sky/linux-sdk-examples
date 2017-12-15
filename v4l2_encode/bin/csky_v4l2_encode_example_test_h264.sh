#!/bin/sh
# For C-SKY eragon - 2017-12-12

if [[ "$#" != 2 ]] ; then
    echo "please enter input and ouput file name!"
    exit 1
fi

INPUT_NAME="$1"
OUTPUT_NAME="$2"
./csky_v4l2_encode_example -c h264 -o $OUTPUT_NAME -i $INPUT_NAME -m /dev/video2 -g 12 -b 6700000 -f 25 -q 30 -p 30
