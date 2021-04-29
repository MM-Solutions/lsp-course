.PHONY = all clean

# Set the compiler tool here.
CC = gcc

# Set linker flags here.
# Only an example. Not necessarily used.
LINKERFLAG = -lm
# Debug level
CFLAGS = -g -Wall

SRC_LIST := $(wildcard *.c)
OBJ := $(SRC_LIST:%.c=%.o)
OUT := $(SRC_LIST:%.c=%)

all: ${OUT}

${OUT}: ${OBJ}
	@echo "Compiling output"
	${CC} ${CFLAGS} $< -o $@ ${LINKERFLAG}

${OBJ}: ${SRC_LIST}
	@echo "Creating object files"
	${CC} ${CFLAGS} -c $<

clean:
	@echo "Cleaning buld products"
	rm -vf *.o ${OUT}
