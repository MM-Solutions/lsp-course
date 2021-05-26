.PHONY = all clean

# Set the compiler tool here
CC = gcc

# Set the linker flags here
LINKERFLAG = -lpthread
# Debug level
CFLAGS = -g -Wall

# List of input source files
SRC_LIST := $(wildcard *.c)
# List of object files
OBJ := $(SRC_LIST:%.c=%.o)
EXEC := test.c
# Name of the target ELF binary
OUT := $(EXEC:%.c=%)

all: ${OUT}

${OUT}: ${OBJ}
	@echo "Compiling output application"
	${CC} ${CFLAGS} $^ -o $@ ${LINKERFLAG}

${OBJ}: ${SRC_LIST}
	@echo "Creating object files"
	${CC} ${CFLAGS} -c $^

clean:
	@echo "Cleaning buld products"
	rm -vf *.o ${OUT}
