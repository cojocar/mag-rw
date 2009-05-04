CFLAGS= -Wall
CC=gcc
LDLIBS=-lrt 

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
BINS := $(patsubst %.c,%,$(SRCS))
all: build

build:$(BINS) 


.PHONY clean:
	rm $(BINS)	
