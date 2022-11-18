
all: aslc

aslc: main.c lexer.c parser.c
	$(CC) $^ -o $@

clean:
	rm -f aslc tmp*
