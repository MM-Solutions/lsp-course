.PHONY = all clean
CC = gcc
LFLAGS = -lm

test: test.o
	@echo "Compiling output"
	${CC} ${LFLAGS} $< -o $@

test.o: test.c test.h
	@echo "Creating object files"
	${CC} -c $<

clean:
	@echo "Cleaning buld products"
	-rm -vf test.o test
