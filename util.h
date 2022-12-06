#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <unistd.h>

#define print(...)                    \
    fprint(stdout, __VA_ARGS__)
#define println(...)                  \
    do {                              \
        fprint(stdout, __VA_ARGS__);  \
        fputc('\n', stdout);          \
    } while (0)
#define eprintln(...)                 \
    do {                              \
        fprint(stderr, __VA_ARGS__);  \
        fputc('\n', stderr);          \
    } while (0)
#define die(...)                      \
    do {                              \
        eprintln(__VA_ARGS__);        \
        exit(EXIT_FAILURE);           \
    } while (0)

void fprint(FILE *file, const char *fmt, ...);
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
int runcmd(char **argv);

#endif // UTIL_H
