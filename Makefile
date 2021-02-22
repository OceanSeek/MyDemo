.PHONY:clean
#itop-4412
#CROSS_COMPILE = /usr/local/arm-2014.05/bin/arm-none-linux-gnueabi-

#imx6ul
#CROSS_COMPILE = /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-none-linux-gnueabi-

#huawei
#CROSS_COMPILE = arm-linux-gnueabi-


#A40i
#CROSS_COMPILE = /root/workspace/allwinner/A40i/bsp/lichee/out/sun8iw11p1/linux/common/buildroot/host/usr/bin/arm-linux-gnueabihf-

#huawei
CROSS_COMPILE = aarch64-linux-gnu-

CC = $(CROSS_COMPILE)gcc #arm
#CC = $(CROSS_COMPILE)g++ #arm
#CC= gcc #x86

LIB_DIR = sqlite3/lib mqtt/lib64
#LIB_DIR = sqlite3/lib

INCLUDE_DIRS =	sqlite3/include \
				mqtt/include \
				src/DBase \
				src/IEC10X \
				src/queue \
				src/sqlite \
				src/usart \
				src/main \
				src/include \
				src/log \
				src/modbus \
				src/mqttclient \
				src/json \
				src/json_test \
				src/protocol/monitor/ \
				src/protocol/master/GX101Master \
				src/protocol/master/GX104Master \
				src/protocol/master/miec104gx \
				src/protocol/master/MXJ103 \
				src/protocol/master/ModbusRtuMaster \
				src/protocol/slaver/GX101Slaver \
				src/protocol/slaver/GX104Slaver \
				src/protocol/slaver/siec101gx_2002 \
				src/protocol/slaver/siec104_2002nr \
				src/protocol/slaver/SXJ103 \
				src/protocol/slaver/SIEC101_2002 \
				src/protocol/slaver/ModbusRtuSlaver \
				
				
#INCLUDE_DIRS =-L./sqlite3/lib -I./sqlite3/include -I./include  -I./src/DBase -I./src/IEC10X -I./src/queue -I./src/sqlite -I./src/usart -I./  #arm
#INCLUDE_DIRS =  -I./src/include  -I./DBase -I./IEC10X -I./queue -I./sqlite -I./  #x86

BIN=WQ-T9001.bin
ROOTSRC=$(wildcard *.c)
ROOTOBJ=$(patsubst %.c, %.o, $(ROOTSRC))
SUBDIR=$(shell ls -d */) $(shell ls ./src -d */)
SUBSRC=$(shell find $(SUBDIR) -name '*.c')
SUBOBJ=$(SUBSRC:%.c=%.o)
#INCLUDE=-L$(LIB_DIR) $(patsubst %, -I %,$(INCLUDE_DIRS))
INCLUDE=$(patsubst %, -L %,$(LIB_DIR)) $(patsubst %, -I %,$(INCLUDE_DIRS))

CFLAGS=$(INCLUDE) -lsqlite3 -pthread -lpaho-mqtt3cs -lm

$(BIN):$(ROOTOBJ) $(SUBOBJ)
	$(CC) $^ -o $@  $(CFLAGS) 
 
%.o:%.c
	$(CC) -c $< -o $@  $(CFLAGS) 

clean:
	rm -rf $(BIN) $(ROOTOBJ) $(SUBOBJ)
