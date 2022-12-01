#ifndef ASLC_H
#define ASLC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define COUNT(A)     (sizeof(A) / sizeof(*(A)))

typedef struct Token Token;
typedef struct BlockScope BlockScope;
typedef struct Scope Scope;
typedef struct Path Path;
typedef struct Node Node;
typedef struct Obj Obj;
typedef struct Type Type;

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
    int64_t ival;
};

Token *tokenize(const char *s);
bool token_eq(const Token *tok, const char *str);
void token_assert(const Token *tok, const char *str);
bool token_consume(Token **tok, const char *str);
void token_assert_consume(Token **tok, const char *str);
bool token_appears_before(const Token *tok, const char *pat, const char *end);

// parser.c

struct BlockScope {
    BlockScope *next;
    Obj *locals;
};

// user named scopes
struct Scope {
    Scope *parent, *children, *next;
    const char *name;
    Obj *globals, *fns;
};

// scope lookup path
struct Path {
    Path *next;
    const char *name;
};

typedef enum {
    NT_NUM,
    NT_VAR,
    NT_ASSIGN,
    NT_ADD,
    NT_SUB,
    NT_MUL,
    NT_DIV,
    NT_DEREF,
    NT_ADDR,
    NT_NEG,
    NT_EQ,
    NT_NE,
    NT_LT,
    NT_LE,
    NT_RET_STMT,
    NT_BLOCK_STMT,
    NT_EXPR_STMT,
    NT_IF_STMT,
    NT_FOR_STMT,
    NT_FN_CALL,
} NodeType;

struct Node {
    NodeType type;
    Node *lch, *rch, *next;
    Type *dt;
    Path *path;
    Scope *scope;
    // number
    int64_t ival;
    // block statement
    Node *block;
    // if statement, for statement
    Node *init, *cond, *iter, *br_if, *br_else;
    // variable
    Obj *var;
    const char *var_name;
    // function call
    const char *fn_name;
    Node *fn_args;
};

typedef enum {
    OT_LOCAL,    // stack automatic variable
    OT_GLOBAL,   // global variable
    OT_FN,       // function
} ObjType;

struct Obj {
    ObjType type;
    Obj *next;
    Type *dt;
    Scope *scope;
    const char *name;
    // local variable
    int rbp_off;
    Obj *bnext;    // next local var in the same blockscope
    // function
    int stack_size;
    Node *body;
    Obj *params, *locals;
};

typedef enum {
    DT_NONE,
    DT_INT,
    DT_PTR,
    DT_FN,
} DataType;

struct Type {
    DataType type;
    Type *next;
    const char *name;
    int size;
    // integer type, float type
    int bits;
    // pointer type
    Type *base;
    // function type
    Type *params;
    Type *ret;
};

Scope *parse(Token *tok);

// codegen.c

void gen(Scope *prog);

// main.c

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

#endif // ASLC_H
