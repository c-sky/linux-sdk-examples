# Demo Description

The demo tests the following rtc function:

* Read Real time value from rtc device
* Set Real time value to rtc device
* Set alarm value to rtc device and test alarm function

# Directory contents
```
+-- bin
|    +--- csky_rtc_example (binary built)
|
+-- csky_rtc_example.c (source code)
|
+-- csky_rtc_test.h (source code)
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
csky_rtc_example 1.0
Usage: csky_rtc_example /dev/rtcN read/write/alarm [datetime]
read: it will give the Realtime clock value
write: it will write datetime into RTC device
alarm: it will make an alarm when time is datetime you given
datetime format：2017-01-01_15:23:45

```

## Example

### 1. Check usage

$ csky_rtc_example

### 1. Example Output

```
csky_rtc_example 1.0
Usage: csky_rtc_example /dev/rtcN read/write/alarm [datetime]
read: it will give the Realtime clock value
write: it will write datetime into RTC device
alarm: it will make an alarm when time is datetime you given
datetime format：2017-01-01_15:23:45

```

### 2. Normal test

#Read Real time clock value
csky_rtc_example read /dev/rtc0

#Example Output
Now it is Year 2017 Month 8 Day 29 Hour 3 Minute 11 Second 26

#Write Real time clock value to rtc device
csky_rtc_example write /dev/rtc0 2017-10-22_21:00:00

#Example Output

#Set Realtime clock Alarm
csky_rtc_example alarm /dev/rtc0 2017-08-29_11:25:00

#Example Output
Alarm Rang!

### Abnormal test

#Set an invalid datetime
csky_rtc_example write /dev/rtc0 2017-02-29_12:00:00

#Example Output
RTC set failed

