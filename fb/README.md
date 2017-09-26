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
version: 1.0
Usage: csky_fb_example path
Where path = framebuffer device name, e.g. /dev/fb0
```

# Example

## 1. Check usage

$ ./csky_fb_example /dev/fb0

## 1. Example Output

```
csky_fb_test  Sep 19 2017  14:52:32
fb res 800x480 virtual 800x960, line_len 3200, bpp 32

---------- main menu ----------
Q - exit
L - enable/disable LCDC
F - set pixel format
G - get pixel format
V - wait for VSYNC
P - pan display(RGB only)
Y - display YUV image(YUV420 only)
Y2 - display YUV image(2 frame)
R - display rectangle(RGB only)
ST - Stress Test
-------------------------------
Input Your Choice:
```

## 2. Display YUV image

$ ./csky_fb_example /dev/fb0
$ Input Y

## 2. Example Output

Check the image on the screen.

## 3. display rectangle in RGB mode

$ ./csky_fb_example /dev/fb0
$ Input R

## 3. Example Output

Check if red rectangle on the screen.
