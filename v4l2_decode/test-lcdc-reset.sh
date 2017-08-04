#!/bin/sh

COUNTER=0

while [ 1 ]
do
	COUNTER=`expr $COUNTER + 1`
	echo "--- count: $COUNTER ---"
	./v4l2_decode -c mpeg4 -d /dev/fb0 -i ./800x480_24fps_allframes.es -m /dev/video3 -t 10
done
