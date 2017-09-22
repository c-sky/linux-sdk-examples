##
 # Copyright 2017 C-SKY Microsystems Co., Ltd.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #   http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
##

include ../Makefile.param

SOURCES = csky_ffmpeg_audio_player_example.c

OBJECTS := $(SOURCES:.c=.o)

ifeq ($(CC),gcc)
EXEC = $(DST_DIR)/csky_ffmpeg_audio_player_example_host
else
EXEC = $(DST_DIR)/csky_ffmpeg_audio_player_example
endif

CFLAGS += -O2

ifeq ($(CC),gcc)
    LFLAGS += -L/usr/local/lib -lavcodec -lavutil -lswresample -lasound -lavformat
else
    LFLAGS += -lavcodec -lavutil -lswresample -lasound -lavformat
endif

all: $(EXEC)

.c.o:
	$(CC) -c $(CFLAGS) $(IFLAGS) $<

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS) $(LFLAGS)

clean:
	rm -f *.o $(EXEC)

install:

.PHONY: clean all
