
all: aslc

aslc: main.c lexer.c
	$(CC) $^ -o $@

clean:
	rm -f aslc tmp*
