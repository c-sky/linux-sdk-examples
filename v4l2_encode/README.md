# Demo Description

The demo tests vpu function in v4l2 framework.

# Directory contents
```
+-- bin
|    +--- csky_v4l2_encode_example (binary built)
|    +--- csky_v4l2_encode_example_test_h264.sh (simple script run csky_v4l2_decode_example)
|    +--- csky_v4l2_encode_example_loop_test_h264.sh (loop test case)
|
+-- csky_v4l2_encode_example.c (main func)
|
+-- args.c (usage / argument parse func)
|
+-- fileops.c (input video file read ops)
|
+-- mfc.c (hardware decode ops)
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
        -i <file> - Input file name
        -m <device> - video decode device (e.g. /dev/video3)
        -V - synchronise to vsync
        -o <file> - Output file name (e.g. ./video.yuv)
        -t <num> - timeout(s)
        -g <num> - GOP size (e.g. 12 default: 16)
        -b <num> - bitrate(bps) (e.g. 6700000)
        -f <num> - frame rate(/s) (e.g. 30 default:25)
        -q <num> - I frame picture quantity (e.g. 12~51 default:30)
        -p <num> - P frame picture quantity (e.g. 12~51 default:30)
```

# Example

## 1. Check usage

$ csky_v4l2_encode_example

## 2. Play video

$ ./csky_v4l2_encode_example_test_h264.sh ./video_1280x720.yuv ./media/test.es

## 2. Example Output

### terminal output
```
V4L2 Coenc encoding example application
input name /mnt/nfs/media/video_1280x720.yuv inst.mfc.name /dev/video2
V4L2 Coenc encoding example application1
i->in.size=207360000, i->in.fd=3
V4L2 Coenc encoding example application2
type=vid-out, width=1280, height=720, pixelformat=YU12, field=none, bytesperline=1280,
        sizeimage=1382400, colorspace=3, flags=0x0, ycbcr_enc=0
create ouptut file '/mnt/nfs/media/test.es' OK
encoding progresss [100%]No frame encoded in last 2 seconds
Daemon thread finished
Threads have finished
```

### file output

Be indicated by user, such as "csky_encode_h264.es"

