
all: aslc

aslc: main.c lexer.c parser.c codegen.c util.c
	$(CC) $^ -o $@

test: aslc util.c FORCE
	$(CC) test/*.c util.c -o test_driver
	./test_driver
	rm -f test_driver tmp*

clean:
	rm -f aslc

FORCE:

.PHONY: aslc test clean FORCE
