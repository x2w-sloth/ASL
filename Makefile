
all: aslc

aslc: main.c lexer.c parser.c type.c codegen.c
	$(CC) $^ -o $@

test: aslc FORCE
	$(CC) test/*.c -o test_driver
	./test_driver
	rm -f test_driver tmp*

clean:
	rm -f aslc

FORCE:

.PHONY: aslc test clean FORCE
