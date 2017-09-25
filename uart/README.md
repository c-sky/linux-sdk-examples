# Demo Description

The demo tests the following iis function:

* Set Uart Baudrate
* Set Data Size


# Directory contents
```
+-- bin
|    +--- csky_uart_example (binary built)
|
+-- csky_uart_example.c (source code)
|
+-- csky_uart_test.h (source code)
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
csky_uart_example 1.0
Usage: csky_uart_example serial_device
serial_device: Serial Device File. for example /dev/ttyS2

```

## Example

### 1. Check usage

$ csky_uart_example

### 1. Example Output

```
csky_uart_example 1.0
Usage: csky_uart_example serial_device
serial_device: Serial Device File. for example /dev/ttyS2

```

### 2. Normal test

#Serial Device Test
csky_uart_example /dev/ttyS2

#Expected Result
It will test the settings of baudrate and data size of UART2

