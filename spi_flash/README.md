# Demo Description

The demo tests spi flash read / write function.

# Directory contents
```
+-- bin
|    +--- csky_spi_flash_example (binary built)
|
+-- csky_spi_flash_example.c (main func)
|
+-- args.c (usage / argument parse func)
|
+-- common.c (common func)
|
+-- spi_flash.c (flash read / write func)
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
Usage:
        ./csky_spi_flash_example
        -d <device>  - SPI flash device (e.g. /dev/mtd0)
        -i           - Show flash informations
        -r           - Read flash
        -w           - Write flash
        -e           - Erase flash
        -o <offset>  - Offset of flash(decimal)
        -l <length>  - Length of operate(decimal)
        -f <file>    - File read into or write from

Examples:
        ./csky_spi_flash_example -d /dev/mtd0 -r -o 1024 -l 64
        ./csky_spi_flash_example -d /dev/mtd0 -r -o 1024 -l 64 -f ./flash.bin
        ./csky_spi_flash_example -d /dev/mtd0 -w -f ./flash.bin
        ./csky_spi_flash_example -d /dev/mtd0 -w -f ./flash.bin -o 512
        ./csky_spi_flash_example -d /dev/mtd0 -w -f ./flash.bin -o 512 -l 1024
        ./csky_spi_flash_example -d /dev/mtd0 -e
        ./csky_spi_flash_example -d /dev/mtd0 -e -o 4096
        ./csky_spi_flash_example -d /dev/mtd0 -e -o 4096 -l 8192
```

# Example

## 1. Check usage

$ csky_spi_flash_example

## 2. Check spi flash device info

$ ./csky_spi_flash_example  -d /dev/mtd0 -i

## 2. Example Output

```
Device '/dev/mtd0' exists and open OK.
  <<      85 usec costed for 'Get MTD device info' >>
mtd info of '/dev/mtd0': 
        type=3('nor-flash'), flags=3072, size=8388608, erasesize=4096, writesize=1, oobsize=0, padding=0
```

## 3. Check spi flash bytes

$ ./csky_spi_flash_example -d /dev/mtd0 -r -o 1024 -l 64

## 3. Example Output
```
Device '/dev/mtd0' exists and open OK.
  <<      83 usec costed for 'Get MTD device info' >>
Reading process: 100%.
  <<     861 usec costed for 'Read 64 bytes test'>>

First 64 bytes are:
        ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
        ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
        ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
        ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 

```

## 4. Erase first 256KB of spi flash

$ ./csky_spi_flash_example -d /dev/mtd0 -e -o 0 -l 2617344

## 4. Example Output
```
Device '/dev/mtd0' exists and open OK.
  <<      85 usec costed for 'Get MTD device info' >>

Erase offset=0, len=2617344? y/N: y
Erase process: 100%.
  <<30157043 usec costed for 'Erase 2617344 bytes'>>
```

## 5.  Write binary to spi flash

$ ./csky_spi_flash_example -d /dev/mtd0 -i -w -f ./uboot-eragon.bin -o 0

## 5. Example Output
```
Device '/dev/mtd0' exists and open OK.
  <<      83 usec costed for 'Get MTD device info' >>
mtd info of '/dev/mtd0': 
        type=3('nor-flash'), flags=3072, size=8388608, erasesize=4096, writesize=1, oobsize=0, padding=0
Write process: 100%.
  << 1226955 usec costed for 'Write 155172 bytes'>>
# ./csky_spi_flash_example  -d /dev/mtd0 -i -r -f read.bin -o 0 -l 155172
Device '/dev/mtd0' exists and open OK.
  <<      88 usec costed for 'Get MTD device info' >>
mtd info of '/dev/mtd0': 
        type=3('nor-flash'), flags=3072, size=8388608, erasesize=4096, writesize=1, oobsize=0, padding=0
Save file 'read.bin' is opened.
Reading process: 100%.
  <<  361200 usec costed for 'Read 155172 bytes and save to file'>>

First 128 bytes are:
        80 01 d0 1f bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f 
        bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f 
        bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f 
        bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f 
        bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f 
        bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f 
        bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f 
        bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f bc 01 d0 1f 
```
