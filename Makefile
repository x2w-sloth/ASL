
all: aslc

aslc: main.c lexer.c parser.c codegen.c
	$(CC) $^ -o $@

clean:
	rm -f aslc tmp*
