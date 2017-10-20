# PWM read/write example application
# Zhang MinFeng <minfeng_zhang@c-sky.com>
#
# Copyright 2017 C-SKY Microsystems Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

TCPATH = csky-abiv2-linux-
CC = ${TCPATH}gcc

SOURCES = lcd_backlight.c

OBJECTS := $(SOURCES:.c=.o)
EXEC = ./bin/csky_lcd_backlight_example
CFLAGS = -O0 -Wall -g -lm

all: $(EXEC)

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS) -pthread

clean:
	rm -f *.o $(EXEC)

install:

.PHONY: clean all
