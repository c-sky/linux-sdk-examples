# Demo Description

The demo tests the following watchdog function:

* set watchdog timeout
* get watchdog timeout
* feed dog timely
* system reset

# Directory contents
```
+-- bin
|    +--- csky_wdt_test (binary built)
|
+-- csky_wdt_test.c (source code)
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
Usage: csky_wdt_example <timeout> <sleep> <mode> <feed times>
    timeout: value in seconds to cause wdt timeout/reset
    sleep: value in seconds to service the wdt
    mode: 0 - Service wdt with ioctl(), 1 - with write()
    feed times: the times of feeding dog
```

## Example

### 1. Check usage

$ csky_wdt_example

### 1. Example Output

```
Usage: csky_wdt_example <timeout> <sleep> <mode> <feed times>
    timeout: value in seconds to cause wdt timeout/reset
    sleep: value in seconds to service the wdt
    mode: 0 - Service wdt with ioctl(), 1 - with write()
    feed times: the times of feeding dog
```

### 2. Normal test (feeddog time < tiemout)

$ csky_wdt_example 10 1 0 2

* watchdog timeout is set to 10s.
* dog is feeded every 1s for 2 times throug ioctl mode.

### 2. Example Output

```
start wdt (timeout: 10, sleep: 1, mode: ioctl, times: 10)
set wdt time out 10 seconds
read back wdt timeout is 10 seconds
feed wdt 1 over
feed wdt 2 over
eragon_bootrom_fpga4:CCTmode
```

### 3. Abnormal test (feeddog time < tiemout)

./csky_wdt_test 2 5 0 5

* watchdog timeout is set to 2s.
* dog is feeded every 5s for 5 times throug ioctl mode.

### 3. Example Output

```
start wdt (timeout: 2, sleep: 5, mode: ioctl, times: 5)
set wdt time out 2 seconds
read back wdt timeout is 2 seconds
eragon_bootrom_fpga4:CCTmode
```
