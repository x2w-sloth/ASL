
all: aslc

aslc: main.c
	$(CC) $^ -o $@

clean:
	rm -f aslc tmp*
