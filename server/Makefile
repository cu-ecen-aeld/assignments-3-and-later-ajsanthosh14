# Makefile to build the aesdsocket application

# Native compilor
CC = gcc

# Cross compiler 
CROSS_COMPILE =

# Compiler flags 
CFLAGS = -g -Wall -Werror

# linkers
LDFLAG = -pthread 

# Source file
SRCS = aesdsocket.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable Target file
TARGET = aesdsocket

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(LDFLAG) -c -o $@ $<

all: $(TARGET)
default: $(TARGET)

$(TARGET): $(OBJS)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(LDFLAG) $^ -o $@


.PHONY: clean

clean:
	$(RM) $(TARGET) *.o *.elf *.map
