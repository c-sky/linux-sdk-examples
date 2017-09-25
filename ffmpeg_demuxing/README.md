# Demo Description

The demo tests the following ffmpeg function:

* demuxing mp4 file and out to the video / audio file

# Directory contents
```
+-- bin
|    +--- csky_ffmpeg_demuxing_example (binary built)
|
+-- csky_ffmpeg_demuxing_example.c (source code)
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
usage: ./bin/csky_ffmpeg_demuxing_example input_file video_output_file audio_output_file
API example program to show how to read frames from an input file.
This program reads frames from a file, demux them and output
the uncoded video frame to video file named video_output_file,
the uncoded audio frame to audio file named video_output_file.

```

# Example

## 1. Check usage

$ csky_ffmpeg_demuxing_example

## 1. Example Output

```
csky_ffmpeg_demuxing_example 1.0
usage: ./bin/csky_ffmpeg_demuxing_example input_file video_output_file audio_output_file
API example program to show how to read frames from an input file.
This program reads frames from a file, demux them and output
the uncoded video frame to video file named video_output_file,
the uncoded audio frame to audio file named video_output_file.
```

## 2. demuxing mp4 file

$ ./bin/csky_ffmpeg_demuxing_example ./bin/SampleVideo_1280x720_1mb.mp4 /tmp/video_out.h264 /tmp/audio_out.aac


## 2. Example Output

```
==============Input Video=============
Input #0, mov,mp4,m4a,3gp,3g2,mj2, from './bin/SampleVideo_1280x720_1mb.mp4':
  Metadata:
    major_brand     : isom
    minor_version   : 512
    compatible_brands: isomiso2avc1mp41
    creation_time   : 1970-01-01T00:00:00.000000Z
    encoder         : Lavf53.24.2
  Duration: 00:00:05.31, start: 0.000000, bitrate: 1589 kb/s
    Stream #0:0(und): Video: h264 (Main) (avc1 / 0x31637661), yuv420p, 1280x720 [SAR 1:1 DAR 16:9], 1205 kb/s, 25 fps, 25 tbr, 12800 tbn, 50 tbc (default)
    Metadata:
      creation_time   : 1970-01-01T00:00:00.000000Z
      handler_name    : VideoHandler
    Stream #0:1(und): Audio: aac (LC) (mp4a / 0x6134706D), 48000 Hz, 5.1, fltp, 384 kb/s (default)
    Metadata:
      creation_time   : 1970-01-01T00:00:00.000000Z
      handler_name    : SoundHandler

==============Output Video============
Output #0, h264, to '/tmp/video_out.h264':
    Stream #0:0: Video: h264 (Main), yuv420p, 1280x720 [SAR 1:1 DAR 16:9], q=2-31, 1205 kb/s

==============Output Audio============
Output #0, adts, to '/tmp/audio_out.aac':
    Stream #0:0: Audio: aac (LC), 48000 Hz, 5.1, fltp, 384 kb/s

```
