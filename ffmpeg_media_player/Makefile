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

SOURCES = audio.c video.c packet_queue.c csky_ffmpeg_media_player.c
SUBDIRS = video
SUBDIR_OBJECTS = $(foreach f, $(SUBDIRS), $(f)/*.o)

OBJECTS := $(SOURCES:.c=.o)
EXEC = $(DST_DIR)/csky_ffmpeg_media_player_example
CFLAGS += -O2
LFLAGS += -lavcodec -lavutil -lswresample -lasound -lavformat -lavfilter

all: $(EXEC)

.c.o:
	$(CC) -c $(CFLAGS) $(IFLAGS) $<

$(SUBDIRS): ECHO
	make -C $@

$(EXEC): $(SUBDIRS) $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS) $(SUBDIR_OBJECTS) $(LFLAGS)

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	rm -f *.o $(EXEC)

ECHO:
	@echo $(SUBDIRS)

install:

.PHONY: clean all
