
all: aslc

aslc: main.c lexer.c parser.c type.c codegen.c
	$(CC) $^ -o $@

clean:
	rm -f aslc tmp*
