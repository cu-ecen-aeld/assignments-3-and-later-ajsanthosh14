#Makfile to build the writer application


#compiler
CC = gcc

#compiler flags
CFLAGS = -g -Wall -Werror

#source files
SRCS = writer.c

#object files
OBJS = $(SRCS:.c=.o)

#Executable Target file
TARGET = writer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

.PHONY : clean

clean:
	$(RM) $(TARGET) *.o

