# Demo Description

The demo tests the following iis function:

* Write File
* Read File
* Write performance test
* Read performance test

# Directory contents
```
+-- bin
|    +--- csky_storage_example (binary built)
|
+-- csky_storage_example.c (source code)
|
+-- csky_storage_test.h (source code)
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
csky_storage_example 1.0
usage: csky_storage_example action filename size [iterations]
action: read, write, readperf, writeperf
filename: name of the file. iteration number will also be added
size: read or write size
iterations: how many times you want to write or read.Default is value is 1, unused for performance test.
When using read action, you should provide a file created by csky_storage_example

```

## Example

### 1. Check usage

$ csky_storage_example

### 1. Example Output

```
csky_storage_example 1.0
usage: csky_storage_example action filename size [iterations]
action: read, write, readperf, writeperf
filename: name of the file. iteration number will also be added
size: read or write size
iterations: how many times you want to write or read.Default is value is 1, unused for performance test.
When using read action, you should provide a file created by csky_storage_example
```

### 2. Normal test

#Write File
csky_storage_example write test 100000 2

#Expected Result
File test-1 and test-2 will be created, and file size is 100000

#Read File
csky_storage_example read test 100000 2

#Expected Result
File test-1 and test-2 will be read and checked.

#Write Performance Test
csky_storage_example writeperf test 100000000

#Example Output
Write Speed 89176 KB/s

#Read Performance Test
csky_storage_example readperf test 100000000

#Example Output
Read Speed 70736 KB/s
