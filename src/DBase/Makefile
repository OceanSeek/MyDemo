
CROSS_COMPILE = /usr/local/arm-2014.05/bin/arm-none-linux-gnueabi-
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
CLIBS = -pthread -lsqlite3
ARFLAG = -rcs
#ARFLAG = crv


LIB_DIR = ./../lib/
TOP_DIR = ./..
 
SOURCES = $(wildcard ./*.c)
INCLUDE_DIRS = -I$(TOP_DIR)/sqlite3/include  -L$(TOP_DIR)/sqlite3/lib -I$(TOP_DIR)/include -I$(TOP_DIR)/IEC10X -I$(TOP_DIR)/queue -I$(TOP_DIR) -I./ 
 
TARGET = libDbase.a
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
 
$(TARGET) : $(OBJECTS)
	$(AR) $(ARFLAG) $@ $^
	cp $@ $(LIB_DIR)
	
$(OBJECTS) : %.o : %.c 
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE_DIRS) $(CLIBS)
 
.PHONY : clean
clean:
	rm -rf $(TARGET) $(OBJECTS)
