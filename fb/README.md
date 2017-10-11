# Demo Description

The demo tests lcdc basic function.

# Directory contents
```
+-- bin
|    +--- csky_fb_example (binary built)
|
+-- csky_fb_example.c (main func)
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
Usage: csky_fb_example [OPTION]
  -d                 framebuffer device name (default /dev/fb0)
  -f                 yuv data from a file (e.g. /media/yuv420_1280x720.yuv)
  -p --pixel-format  pixel format (rgb or yuv420, default rgb)
  --hdmi             display image via HDMI
```

# Example

## 1. Display YUV image via LCD

$ ./csky_fb_example -d /dev/fb0 -p yuv420 -f /example/media/yuv420_800x480.yuv

## 2. display rectangle via LCD

$ ./csky_fb_example -d /dev/fb0

## 3. Display YUV image via HDMI

$ ./csky_fb_example -d /dev/fb0 -p yuv420 -f /example/media/yuv420_1280x720.yuv --hdmi

