.PHONY = all clean

CC = gcc
LFLAGS = -lm

SRC_LIST := $(wildcard *.c)
OBJ := $(SRC_LIST:%.c=%.o)
OUT := $(SRC_LIST:%.c=%)

all: ${OUT}
