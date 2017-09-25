# Demo Description

The demo is used to play multimedia file.

# Directory contents
```
+-- bin
|    +--- csky_ffmpeg_media_player_example (binary built)
|
+-- video/ (v4l2_decode source code)
|
+-- csky_ffmpeg_media_player_example.c (source code)
|
+-- audio.c (audio decode file)
|
+-- video.c (video context setup and get video packet)
|
+-- packet_queue.c (queue audio / video packet for processing)
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
        ././csky_ffmpeg_media_player_example
        -c <codec> - The codec of the encoded stream
                     Available codecs: mpeg4, h264
        -d <device>  - Frame buffer device (e.g. /dev/fb0)
        -i <file> - Input file name
        -m <device> - video decode device (e.g. /dev/video3)
        -V - synchronise to vsync
        -r <file> - Record file name (e.g. ./video.yuv)
```

# Example

## 1. Check usage

$ csky_ffmpeg_media_player_example

## 2. Play multimedia file

$ csky_ffmpeg_media_player_example -c mpeg4 -i ./test.mp4 -m /dev/video3 -d /dev/fb0


## 2. Example Output

```
Input #0, mov,mp4,m4a,3gp,3g2,mj2, from './test.mp4':
  Metadata:
    major_brand     : isom
    minor_version   : 512
    compatible_brands: isomiso2mp41
    encoder         : Lavf57.71.100
  Duration: 00:01:02.35, start: 0.000000, bitrate: 1433 kb/s
    Stream #0:0(und): Video: mpeg4 (Advanced Simple Profile) (mp4v / 0x7634706D), yuv420p, 800x480 [SAR 16:15 DAR 16:9], 1367 kb/s, 25 fps, 25 tbr, 12800 tbn, 25 tbc (default)
    Metadata:
      handler_name    : VideoHandler
    Stream #0:1(und): Audio: mp3 (mp4a / 0x6134706D), 48000 Hz, mono, s16p, 64 kb/s (default)
    Metadata:
      handler_name    : SoundHandler
V4L2 Codec decoding example application
type=vid-cap, width=1920, height=1088, pixelformat=YU12, field=none, bytesperline=1920, 
        sizeimage=3133440, colorspace=3, flags=0x0, ycbcr_enc=0, quantization=0, xfer_func=0
[I]audio.c:48: AV_SAMPLE_FMT_S16P
[I]audio.c:113: channels=1
[I]audio.c:116: sample_rate=48000
[I]audio.c:138: snd_pcm_get_params, buffer_size=16384, period_size=256
[I]audio.c:158: snd_pcm_sw_params_get_avail_min, val=256
[I]audio.c:168: snd_pcm_sw_params_get_start_threshold, val=1
[I]audio.c:177: snd_pcm_sw_params_set_start_threshold, val=16384
[mp3 @ 0x2d800480] Could not update timestamps for skipped samples.
[I]audio.c:259: frame: channnels=1, default_layout=4, format=6, sample_rate=48000
ALSA lib pcm.c:8323:(snd_pcm_recover) underrun occurred
ALSA lib pcm_hw.c:632:(snd_pcm_hw_start) SNDRV_PCM_IOCTL_START failed (-16): Device or resource busy
[E]audio.c:357: snd_pcm_writei failed, ret=-16
[I]csky_ffmpeg_media_player.c:122: finish reading.delay...
[I]csky_ffmpeg_media_player.c:124: Play finished!
Parser thread finished
ALSA lib pcm_hw.c:661:(snd_pcm_hw_drain) SNDRV_PCM_IOCTL_DRAIN failed (-5): Input/output error
No frame decoded in last 2 seconds
Daemon thread finished
Threads have finished
```
