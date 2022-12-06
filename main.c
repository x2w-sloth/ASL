#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "aslc.h"

_Static_assert (sizeof(int) == 4, "platform int is not 32-bit");

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
