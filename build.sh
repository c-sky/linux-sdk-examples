#!/bin/sh

if [ "$1" = "clean" ]
then
	make CSKY_EXAMPLES_BUILD_ALL=y clean
else
	make CSKY_EXAMPLES_BUILD_ALL=y
fi
