#!/bin/sh
# For C-SKY eragon - 2017-05-27
#./v4l2_decode -c mpeg4 -d /dev/fb0 -i ./800x480_24fps.es -m /dev/video3
while [ 1 ]
do
	./v4l2_decode -c mpeg4 -d /dev/fb0 -i ./800x480_24fps_allframes.es -m /dev/video3
done
