#!/bin/sh

if [[ "$#" != 2 ]] ; then
    echo "please enter input and output file name!"
    exit 1
fi

INPUT_NAME="$1"
OUTPUT_NAME="$2"

COUNTER=0

while [ 1 ]
do
	COUNTER=`expr $COUNTER + 1`
	echo "--- count: $COUNTER ---"
	./csky_v4l2_encode_example -c h264 -o $OUTPUT_NAME -i $INPUT_NAME -m /dev/video2 -t 6 -g 12 -b 6700000 -f 25 -q 30 -p 30
done
