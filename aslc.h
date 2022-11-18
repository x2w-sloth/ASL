#ifndef ASLC_H
#define ASLC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct Token Token;
typedef struct Node Node;

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

// parser.c

typedef enum {
    NT_NUM
} NodeType;

struct Node {
    NodeType type;
    Node *lch, *rch;
    // number
    int ival;
};

Node *parse(Token *tok);

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
