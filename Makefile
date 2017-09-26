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

SUBDIRS := 	watchdog \
		fb \
		spi_flash \
		v4l2_decode \
		storage \
		timer \
		pwm \
		network \
		rtc \
		v4l2_decode \
		uart

ifeq ($(BR2_PACKAGE_LIBSNDFILE),y)
SUBDIRS +=	iis
endif

ifeq ($(BR2_PACKAGE_FFMPEG),y)
SUBDIRS +=	ffmpeg_audio_player \
		ffmpeg_media_player \
		ffmpeg_decode_audio \
		ffmpeg_demuxing
endif

all: $(SUBDIRS)
$(SUBDIRS):
	echo $(BR2_PACKAGE_FFMPEG)
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

doc:
	for dir in $(SUBDIRS); do \
		pandoc $$dir/README.md -o $$dir/README.html; \
	done

.PHONY: all $(SUBDIRS) clean
