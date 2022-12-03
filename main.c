#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "aslc.h"

_Static_assert (sizeof(int) == 4, "platform int is not 32-bit");

void
fprint(FILE *file, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(file, fmt, ap);
    va_end(ap);
}

void *
xmalloc(size_t size)
{
    void *mem = malloc(size);

    if (!mem)
        die("malloc failed to allocate %u bytes\n", size);

    return mem;
}

void *
xrealloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);

    if (!ptr)
        die("realloc failed to resize to %u\n", size);

    return ptr;
}

static char *
file_buf(const char *path)
{
    char tmp[BUFSIZ];
    char *buf = xmalloc(BUFSIZ);
    size_t buf_size = 1;
    FILE *file;

    buf[0] = '\0';

    if (!strcmp(path, "-")) // read from stdin
        file = stdin;
    else
        file = fopen(path, "r");

    if (!file)
        die("failed to read file %s", path);

    while (fgets(tmp, BUFSIZ, file))
    {
        buf_size += BUFSIZ;
        buf = xrealloc(buf, buf_size);
        strncat(buf, tmp, BUFSIZ);
    }

    if (file != stdin)
        fclose(file);

    return buf;
}

int
main(int argc, char **argv)
{
    if (argc != 2)
        die("usage: %s <source>", argv[0]);
    
    char *buf = file_buf(argv[1]);

    Token *tok = tokenize(buf);
    Scope *prog = parse(tok);
    gen(prog);

    return 0;
}
