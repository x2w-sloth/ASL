#ifndef ASLC_H
#define ASLC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct Token Token;
typedef struct Node Node;

// lexer.c

typedef enum {
    TT_NUM,      // numeric literal
    TT_PUNC,     // punctuator
    TT_END,      // last token
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
bool token_eq(const Token *tok, const char *str);
void token_assert(const Token *tok, const char *str);
bool token_consume(Token **tok, const char *str);

// parser.c

typedef enum {
    NT_NUM,
    NT_ADD,
    NT_SUB,
    NT_MUL,
    NT_DIV,
    NT_NEG,
} NodeType;

struct Node {
    NodeType type;
    Node *lch, *rch;
    // number
    int ival;
};

Node *parse(Token *tok);

// codegen.c

void gen(Node *node);

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
