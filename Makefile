VPATH = src
CC = gcc
CFLAGS  = -std=c11 -Wall -Werror
TARGET = fractal
DEPS = $(TARGET).h algorithms.h performance.h bitmap.h testing.h
SRC = $(TARGET).c algorithms.c  performance.c bitmap.c testing.c
LIBS = -lm # benötigt für math.h weil nicht Teil der C-Standardlib

$(TARGET): $(SRC)
	$(CC) -o bin/fractal $^ $(CFLAGS) $(LIBS) 

.PHONY: clean

clean:
	$(RM) $(TARGET) *.o
