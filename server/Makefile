# Makefile to build the aesdsocket application

# Native compilor
CC = gcc

# Compiler flags 
CFLAGS = -g -Wall -Werror

# linkers
LDFLAG = -pthread -lrt 

# Source file
SRCS = aesdsocket.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable Target file
TARGET = aesdsocket

all: $(TARGET)
default: $(TARGET)

$(TARGET): $(OBJS)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(LDFLAG) $^ -o $@


.PHONY: clean

clean:
	$(RM) $(TARGET) *.o *.elf *.map
