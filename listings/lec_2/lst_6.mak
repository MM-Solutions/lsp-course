.PHONY = all clean

# The default target relates to the
# test executable we want to generate
all: test

# Rule to compile the test executable
test: test.c test.h
	@echo "Compiling output"
	gcc -o test -lm test.c test.h

# Rule to clean the generated files
clean:
	@echo "Cleaning the test app"
	-rm -f test test.o
