#Makfile to build the writer application

#native compiler
CC = gcc

#compiler flags
CFLAGS = -g -Wall -Werror

#source files
SRCS = writer.c

#object files
OBJS = $(SRCS:.c=.o)

#Executable Target file
TARGET = writer


%.o: %.c
	$(CROSS_COMPILE)-$(CC) $(CFLAGS) -c -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CROSS_COMPILE)-$(CC) $(CFLAGS) $^ -o $@


.PHONY: clean

clean:
	$(RM) $(TARGET) *.o *.elf *.map

