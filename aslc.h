#ifndef ASLC_H
#define ASLC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#define COUNT(A)     (sizeof(A) / sizeof(*(A)))

typedef struct Token Token;
typedef struct Node Node;
typedef struct Obj Obj;

// lexer.c

typedef enum {
    TT_NUM,      // numeric literal
    TT_PUNC,     // punctuator
    TT_IDENT,    // identifier
    TT_KEYWORD,  // keyword
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
void token_assert_consume(Token **tok, const char *str);

// parser.c

typedef enum {
    NT_NUM,
    NT_VAR,
    NT_ASSIGN,
    NT_ADD,
    NT_SUB,
    NT_MUL,
    NT_DIV,
    NT_NEG,
    NT_EQ,
    NT_NE,
    NT_LT,
    NT_LE,
    NT_RET_STMT,
    NT_BLOCK_STMT,
    NT_EXPR_STMT,
    NT_FN_CALL,
} NodeType;

struct Node {
    NodeType type;
    Node *lch, *rch, *next;
    // number
    int ival;
    // block statement
    Node *block;
    // variable
    Obj *var;
    // function call
    const char *fn_name;
};

typedef enum {
    OT_LOCAL,    // stack automatic variable
    OT_FN,       // function
} ObjType;

struct Obj {
    ObjType type;
    Obj *next;
    const char *name;
    // local variable
    int rbp_off;
    // function
    int stack_size;
    Node *body;
    Obj *locals;
};

Obj *parse(Token *tok);

// codegen.c

void gen(Obj *prog);

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
