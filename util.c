#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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
        die("malloc failed to allocate %zu bytes\n", size);

    return mem;
}

void *
xrealloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);

    if (!ptr)
        die("realloc failed to resize to %zu\n", size);

    return ptr;
}

int
runcmd(char **argv)
{
    int status;
    pid_t child;

    child = fork();
    if (child == 0)
    {
        // child process exits after executing cmd
        execvp(argv[0], argv);
        die("execvp failed");
    }
    else
    {
        // wait for child process to finish
        while (wait(&status) != child) ;
    }

    return status;
}
