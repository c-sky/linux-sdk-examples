# Demo Description

The demo tests the following iis function:

* Play wave file(Including get wave file information)
* Tune volume


# Directory contents
```
+-- bin
|    +--- csky_iis_example (binary built)
|
+-- csky_iis_example.c (source code)
|
+-- csky_iis_test.h (source code)
|
+-- Makefile
|
+-- README.md (this file)
```
# How to build

* Before building the demo, please set the toolchain path to PATH global var.
* And you must make sure libsndfile and libasound are already built
* Please set libsndfile and libasound's path  into  LD_LIBRARY_PATH
* Use `make` to build the example, the binary will be created in bin/ dir.
* Use `make clean` to clean the example binary.

# How to use

```
csky_iis_example 1.0
Usage: csky_iis_example show/play file [volume] 
show: it will show the wave file information
play: it will play the wave file as a media player
file: {string} A media file in wave format.
volume: {integer} volume value, range(0-100]

```

## Example

### 1. Check usage

$ csky_iis_example

### 1. Example Output

```
csky_iis_example 1.0
Usage: csky_iis_example show/play file [volume] 
show: it will show the wave file information
play: it will play the wave file as a media player
file: {string} A media file in wave format.
volume: {integer} volume value, range(0-100]

```

### 2. Normal test

#Get wave file information
csky_iis_example show somename.wav

#Example Output
Wave File informations:
Frames: 3080532
Channels: 2
Sample rate: 44100
Bits: Signed 16 bit

#Play wave file
csky_iis_example play somename.wav

#Example Output
Frames: 3080532
Channels: 2
Sample rate: 44100
Bits: Signed 16 bit
underrun occurred
end of file on input
End read.

#Tune volume before playing wave file
#You should feel the  volume is different
csky_iis_example play somename.wav 50

#Example Output
Frames: 3080532
Channels: 2
Sample rate: 44100
Bits: Signed 16 bit
underrun occurred
short read: read 66 bytes
end of file on input
End read.


