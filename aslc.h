#ifndef ASLC_H
#define ASLC_H

#include <stdio.h>
#include <stdarg.h>

typedef struct Token Token;

// lexer.c

typedef enum {
    TT_NUM,
} TokenType;

struct Token {
    TokenType type;
    Token *next;
    const char *pos;
    size_t len;
    // numeric literal
    int ival;
};

Token *tokenize(const char *s);

// main.c

#define println(...)    fprintln(stdout, __VA_ARGS__)
#define eprintln(...)   fprintln(stderr, __VA_ARGS__)
#define die(...)               \
    do {                       \
        eprintln(__VA_ARGS__); \
        exit(EXIT_FAILURE);    \
    } while (0)

void fprintln(FILE *file, const char *fmt, ...);
void *xmalloc(size_t size);

#endif // ASLC_H
