# Demo Description

The demo tests vpu function in v4l2 framework.

# Directory contents
```
+-- bin
|    +--- csky_v4l2_decode_example (binary built)
|    +--- test.sh (simple script run csky_v4l2_decode_example)
|    +--- test-lcdc-reset.sh (loop test case)
|
+-- csky_v4l2_decode_example.c (main func)
|
+-- args.c (usage / argument parse func)
|
+-- fb.c (framebuffer ops)
|
+-- fileops.c (input video file read ops)
|
+-- mfc.c (hardware decode ops)
|
+-- parser.c (parse stream and feed MFC with frames to decode)
|
+-- v4l2_common.c (v4l2 print func)
|
+-- Makefile
|
+-- README.md (this file)
```

# How to build

* Before building the demo, please set the toolchain path to PATH global var.
* Use `make` to build the example, the binary will be created in bin/ dir.
* Use `make clean` to clean the example binary.

# How to use

```
Usage:
        ././csky_v4l2_decode_example
        -c <codec> - The codec of the encoded stream
                     Available codecs: mpeg4, h264
        -d <device>  - Frame buffer device (e.g. /dev/fb0)
        -i <file> - Input file name
        -m <device> - video decode device (e.g. /dev/video3)
        -V - synchronise to vsync
        -r <file> - Record file name (e.g. ./video.yuv)
        p2
```

# Example

## 1. Check usage

$ csky_v4l2_decode_example

## 2. Play video

$ ./csky_v4l2_decode_example -c mpeg4 -d /dev/fb0 -i ./800x480_24fps_allframes.es -m /dev/video3

## 2. Example Output

### terminal output
```
V4L2 Codec decoding example application
i->in.size=48997467, i->in.fd=3
type=vid-cap, width=1920, height=1088, pixelformat=YU12, field=none, bytesperline=1920, 
        sizeimage=3133440, colorspace=3, flags=0x0, ycbcr_enc=0, quantization=0, xfer_func=0
Parser thread finished
No frame decoded in last 2 seconds
Daemon thread finished
Threads have finished
```

### LCD output

The video will be played in the LCD screen.

