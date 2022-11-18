#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "aslc.h"

_Static_assert (sizeof(int) == 4, "platform int is not 32-bit");

void
fprintln(FILE *file, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(file, fmt, ap);
    va_end(ap);
    fputc('\n', file);
}

void *
xmalloc(size_t size)
{
    void *mem = malloc(size);

    if (!mem)
        die("malloc failed to allocate %u bytes\n", size);

    return mem;
}

int
main(int argc, char **argv)
{
    if (argc != 2)
        die("usage: %s <source>", argv[0]);

    Token *tok = tokenize(argv[1]);
    Node *root = parse(tok);
    gen(root);

    return 0;
}
