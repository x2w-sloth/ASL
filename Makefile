CCFLAGS = -Wall

all: aslc

aslc: main.c lexer.c parser.c codegen.c util.c
	$(CC) $(CCFLAGS) $^ -o $@

test: aslc util.c FORCE
	$(CC) $(CCFLAGS) test/*.c util.c -o test_driver
	./test_driver
	rm -f test_driver tmp*

clean:
	rm -f aslc

FORCE:

.PHONY: aslc test clean FORCE
