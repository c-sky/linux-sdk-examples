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

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_WATCHDOG)),)
SUBDIRS +=	watchdog
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_FB)),)
SUBDIRS +=	fb
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_SPI_FLASH)),)
SUBDIRS +=	spi_flash
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_STORAGE)),)
SUBDIRS +=	storage
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_TIMER)),)
SUBDIRS +=	timer
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_PWM)),)
SUBDIRS +=	pwm
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_NETWORK)),)
SUBDIRS +=	network
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_RTC)),)
SUBDIRS +=	rtc
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_UART)),)
SUBDIRS +=	uart
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_V4L2_DECODE)),)
SUBDIRS +=	v4l2_decode
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_V4L2_ENCODE)),)
SUBDIRS +=	v4l2_encode
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_IIS)),)
ifeq ($(BR2_PACKAGE_LIBSNDFILE),y)
SUBDIRS +=	iis
endif
endif

ifneq ($(filter y, $(CSKY_EXAMPLES_BUILD_ALL) $(BR2_PACKAGE_CSKY_EXAMPLES_FFMPEG_PLAYER)),)
ifeq ($(BR2_PACKAGE_FFMPEG),y)
SUBDIRS +=	ffmpeg_audio_player \
		ffmpeg_media_player \
		ffmpeg_decode_audio \
		ffmpeg_demuxing
endif
endif

all: prebuild $(SUBDIRS)
$(SUBDIRS):
	echo $(BR2_PACKAGE_FFMPEG)
	$(MAKE) -C $@
ifneq ($(CSKY_EXAMPLES_TARGET_DIR),)
	cp -rf $@/bin/* $(CSKY_EXAMPLES_TARGET_DIR)
endif

prebuild:
ifneq ($(CSKY_EXAMPLES_TARGET_DIR),)
	mkdir -p $(CSKY_EXAMPLES_TARGET_DIR)
	rm -rf $(CSKY_EXAMPLES_TARGET_DIR)/*
endif

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

doc:
	for dir in $(SUBDIRS); do \
		pandoc $$dir/README.md -o $$dir/README.html; \
	done

.PHONY: all $(SUBDIRS) clean prebuild
