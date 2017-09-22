# Demo Description

The demo tests the following ffmpeg function:

* decode mp3 file and out to the pcm audio file


# Directory contents
```
+-- bin
|    +--- csky_ffmpeg_decode_audio_example (binary built)
|
+-- csky_ffmpeg_decode_audio_example.c (source code)
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
Usage: ./csky_ffmpeg_decode_audio_example <input file> <output file>
```

# Example

## 1. Check usage

$ csky_ffmpeg_decode_audio_example

## 1. Example Output

```
./csky_ffmpeg_decode_audio_example 1.0
Usage: ./csky_ffmpeg_decode_audio_example <input file> <output file>
```

## 2. Normal test

$ ./csky_ffmpeg_decode_audio_example test.mp3 test.pcm


## 2. Example Output

```
test.mp3 is converted to test.pcm successfully!
```


## 3. Abnormal test (input file is not mp3 file)

$ csky_ffmpeg_decode_audio_example test.wav test.pcm

## 3. Example Output

```
[mp3 @ 0x1d65300] Header missing
Error submitting the packet to the decoder
```
