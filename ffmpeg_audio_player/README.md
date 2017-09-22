# Demo Description

The demo is used to play audio file.

# Directory contents
```
+-- bin
|    +--- csky_ffmpeg_audio_player_example (binary built)
|
+-- csky_ffmpeg_audio_player_example.c (source code)
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
Usage: ./csky_ffmpeg_audio_player_example <input file>
```

# Example

## 1. Check usage

$ csky_ffmpeg_audio_player_example

## 2. Play mp3 file

$ ./csky_ffmpeg_audio_player_example test.mp3


## 2. Example Output

```
Input #0, mp3, from './bin/test.mp3':
  Duration: 00:00:04.73, start: 0.000000, bitrate: 32 kb/s
    Stream #0:0: Audio: mp3, 22050 Hz, mono, s16p, 32 kb/s
[I]csky_ffmpeg_audio_player_example.c:433: audio player play successfully!
```
