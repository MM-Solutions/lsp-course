.DEFAULT_GOAL := create
.PHONY: all hello create clean

all: hello

hello:
	@echo "Hello Make"

create:
	@echo "Create a .c file, if it does not exist!"
	touch test.c

clean:
	@echo "Cleaning test.c file"
	rm *.c
