#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <unistd.h>

#define eprintf(...)                  \
        fprintf(stderr, __VA_ARGS__)
#define die(...)                      \
    do {                              \
        eprintf(__VA_ARGS__);         \
        fputc('\n', stderr);          \
        exit(EXIT_FAILURE);           \
    } while (0)

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
int runcmd(char **argv);

#endif // UTIL_H
