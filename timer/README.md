# Demo Description

The demo tests the following iis function:

* Test timer timeout function

# Directory contents
```
+-- bin
|    +--- csky_timer_example (binary built)
|
+-- csky_timer_example.c (source code)
|
+-- csky_timer_test.h (source code)
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
csky_timer_example 1.0
Usage: csky_timer_example timercount timeout
timercount: how many timer you need to create
timeout: timeout value.

```

## Example

### 1. Check usage

$ csky_timer_example

### 1. Example Output

```
csky_timer_example 1.0
Usage: csky_timer_example timercount timeout
timercount: how many timer you need to create
timeout: timeout value.

```

### 2. Normal test

#Test timer timeout
csky_timer_example 2  11

#Example Output
There are still 5 seconds to wait.
Timer has expired.
Timer has expired.
All the timers should timeout
