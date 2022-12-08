#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "aslc.h"

_Static_assert (sizeof(int) == 4, "platform int is not 32-bit");

static char *file_buf();
static char *swap_ext(char *name, char *ext);
static void args(int argc, char **argv);

// global configuration
Config cfg;

static char *
file_buf()
{
    char tmp[BUFSIZ];
    char *buf = xmalloc(BUFSIZ);
    size_t buf_size = 1;
    FILE *file;

    buf[0] = '\0';

    file = cfg.read_stdin ? stdin : fopen(cfg.srcfile, "r");
    if (!file)
        die("failed to read file %s", cfg.srcfile);

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

static char *
swap_ext(char *name, char *ext)
{
    char *str, *p = NULL;
    int n;

    // NOTE: we do not account for multi extensions
    // everything after first '.' is replaced with new extension
    // exception: handle paths like ./source.asl where the first char is '.'
    p = strchr((name[0] == '.') ? name + 1 : name, '.');
    if (!p)
        p = name + strlen(name);

    if (!ext)
        return strndup(name, p - name);

    n = p - name;
    str = (char *)xmalloc(n + strlen(ext) + 2);
    strncpy(str, name, n);
    str[n] = '.';
    strncpy(str + n + 1, ext, strlen(ext));
    str[n + strlen(ext) + 2] = '\0';

    return str;
}

static void
args(int argc, char **argv)
{
    int i;

    if (argc == 1)
        die("usage: %s <source.asl>", argv[0]);

    for (i = 1; i < argc; i++)
    {
        // specify output name
        if (!strcmp(argv[i], "-o"))
        {
            if (i + 1 >= argc)
                die("specified -o option but missing output name");
            cfg.outfile = argv[i + 1];
            i++;
            continue;
        }
        // specify input source
        if (!cfg.read_stdin && !cfg.srcfile)
        {
            if (!strcmp(argv[i], "-"))
                cfg.read_stdin = true;
            else
                cfg.srcfile = argv[i];
        }
    }

    if (!cfg.read_stdin && !cfg.srcfile)
        die("no input source");
    if (cfg.read_stdin && !cfg.outfile)
        die("specify output file with \"-o\" if reading from stdin");

    char *base  = swap_ext(cfg.outfile ? cfg.outfile : cfg.srcfile, NULL);
    cfg.genfile = swap_ext(base, "S");
    cfg.objfile = swap_ext(base, "o");
    cfg.binfile = base;
}

int
main(int argc, char **argv)
{
    args(argc, argv);
    
    char *buf = file_buf(cfg.srcfile);
    Token *tok = tokenize(buf);
    Scope *prog = parse(tok);
    gen(prog);

    // assemble and link program
    char *asm_cmd[]  = { "as", cfg.genfile, "-o", cfg.objfile, "-msyntax=intel", "-mnaked-reg", NULL };
    if (runcmd(asm_cmd) != 0)
        die("failed to assemble");

    char *link_cmd[] = { "ld", cfg.objfile, "-o", cfg.binfile, NULL };
    if (runcmd(link_cmd) != 0)
        die("failed to link");

    return 0;
}
