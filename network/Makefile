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

SET_TARGET = csky_netsetting_example
TRANS_TARGET = csky_nettrans_example

all:
	$(CC) $(CFLAGS) $(IFLAGS) -o $(DST_DIR)/$(SET_TARGET) $(SET_TARGET).c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(DST_DIR)/$(TRANS_TARGET) $(TRANS_TARGET).c
clean:
	rm -f $(DST_DIR)/$(SET_TARGET) $(DST_DIR)/$(TRANS_TARGET)

