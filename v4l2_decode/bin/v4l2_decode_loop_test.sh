#!/bin/sh

if [[ "$#" != 1 ]] ; then
    echo "please enter file name!"
    exit 1
fi

NAME="$1"

COUNTER=0

while [ 1 ]
do
	COUNTER=`expr $COUNTER + 1`
	echo "--- count: $COUNTER ---"
	./csky_v4l2_decode_example -c mpeg4 -d /dev/fb0 -i $NAME -m /dev/video3 -t 10
done
