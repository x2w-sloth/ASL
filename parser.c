#include <string.h>
#include "aslc.h"

static Node *new_node(NodeType type);
static Node *new_binary(NodeType type, Node *lch, Node *rch);
static Node *new_unary(NodeType type, Node *lch);
static Node *node_var(Obj *var);
static Node *node_num(Token *tok);
static Obj *new_obj(ObjType type);
static Obj *obj_local(const char *name);
static Obj *obj_fn(const char *name);
static Obj *find_local(const char *name, size_t len);
static Obj *parse_fn(Token **now);
static Node *parse_decl(Token **now);
static Type *parse_declspec(Token **now);
static Node *parse_declarator(Token **now);
static Node *parse_stmt(Token **now);
static Node *parse_block_stmt(Token **now);
static Node *parse_expr_stmt(Token **now);
static Node *parse_expr(Token **now);
static Node *parse_assign(Token **now);
static Node *parse_cmp(Token **now);
static Node *parse_add(Token **now);
static Node *parse_mul(Token **now);
static Node *parse_unary(Token **now);
static Node *parse_primary(Token **now);

static Obj *locals;

Obj *
parse(Token *tok)
{
    Obj dummy = {};
    Obj *obj = &dummy;

    // parse program as one or more function definitions
    while (tok->type != TT_END)
        obj = obj->next = parse_fn(&tok);

    return dummy.next;
}

static Node *
new_node(NodeType type)
{
    Node *node = xmalloc(sizeof(Node));
    memset(node, 0, sizeof(Node));

    node->type = type;

    return node;
}

static Node *
new_binary(NodeType type, Node *lch, Node *rch)
{
    Node *node = new_node(type);

    node->lch = lch;
    node->rch = rch;

    return node;
}

static Node *
new_unary(NodeType type, Node *lch)
{
    Node *node = new_node(type);

    node->lch = lch;

    return node;
}

static Node *
node_var(Obj *var)
{
    Node *node = new_node(NT_VAR);

    node->var = var;

    return node;
}

static Node *
node_num(Token *tok)
{
    Node *node = new_node(NT_NUM);
    node->ival = tok->ival;

    return node;
}

static Obj *
new_obj(ObjType type)
{
    Obj *obj = xmalloc(sizeof(Obj));
    memset(obj, 0, sizeof(Obj));

    obj->type = type;

    return obj;
}

static Obj *
obj_local(const char *name)
{
    Obj *obj = new_obj(OT_LOCAL);

    obj->name = name;
    obj->next = locals;
    locals = obj;

    return obj;
}

static Obj *
obj_fn(const char *name)
{
    Obj *obj = new_obj(OT_FN);

    obj->name = name;

    return obj;
}

static Obj *
find_local(const char *name, size_t len)
{
    for (Obj *obj = locals; obj; obj = obj->next)
        if (strlen(obj->name) == len && !strncmp(name, obj->name, len))
            return obj;
    return NULL;
}

// <fn> = "fn" <ident> "(" ")" "{" <block_stmt>
static Obj *parse_fn(Token **now)
{
    Token *tok = *now;

    token_assert_consume(&tok, "fn");
    if (tok->type != TT_IDENT)
        die("expected function name identifier");

    Obj *fn = obj_fn(strndup(tok->pos, tok->len));
    tok = tok->next;

    token_assert_consume(&tok, "(");
    token_assert_consume(&tok, ")");
    token_assert_consume(&tok, "{");
    locals = NULL;
    fn->body = parse_block_stmt(&tok);
    fn->locals = locals;

    *now = tok;
    return fn;
}

// <decl> = <declspec> <declarator>
static Node *
parse_decl(Token **now)
{
    Token *tok = *now;

    Type *ty = parse_declspec(&tok);

    Node *node = new_node(NT_BLOCK_STMT);
    node->block = parse_declarator(&tok);

    *now = tok;
    return node;
}

// <declspec> = "i64"
static Type *
parse_declspec(Token **now)
{
    token_assert_consume(now, "i64");
    return &type_i64;
}

// <declarator> = <ident> ("=" <expr>)? ("," <ident> ("=" <expr>)?)* ";"
static Node *
parse_declarator(Token **now)
{
    Token *tok = *now;
    Node dummy = {};
    Node *node = &dummy;

    while (!token_eq(tok, ";"))
    {
        token_consume(&tok, ",");

        if (tok->type != TT_IDENT)
            die("expected identifier in declarator, got %d", tok->type);
        if (find_local(tok->pos, tok->len))
            die("identifier %.*s already declared", tok->len, tok->pos);

        Obj *local = obj_local(strndup(tok->pos, tok->len));

        tok = tok->next;
        if (!token_consume(&tok, "="))
            continue;

        // assign value on declaration
        Node *lch = node_var(local);
        Node *rch = parse_assign(&tok);
        node = node->next = new_unary(NT_EXPR_STMT, new_binary(NT_ASSIGN, lch, rch));
    }

    *now = tok->next;
    return dummy.next;
}

// <stmt> = "{" <block_stmt>
//        | "return" <expr_stmt>
//        | <expr_stmt>
static Node *
parse_stmt(Token **now)
{
    if (token_consume(now, "{"))
        return parse_block_stmt(now);
    if (token_consume(now, "return"))
        return new_unary(NT_RET_STMT, parse_expr_stmt(now));

    return parse_expr_stmt(now);
}

// <block_stmt> = (<decl> | <stmt>)* "}"
static Node *
parse_block_stmt(Token **now)
{
    Token *tok = *now;
    Node dummy = {};
    Node *node = &dummy;

    while (!token_eq(tok, "}"))
    {
        if (token_eq(tok, "i64"))
            node = node->next = parse_decl(&tok);
        else
            node = node->next = parse_stmt(&tok);
    }

    node = new_node(NT_BLOCK_STMT);
    node->block = dummy.next;
    *now = tok->next;
    return node;
}

// <expr_stmt> = (<expr>)? ";"
static Node *
parse_expr_stmt(Token **now)
{
    Node *node;

    if (token_consume(now, ";"))
        return new_node(NT_BLOCK_STMT);

    node = new_unary(NT_EXPR_STMT, parse_expr(now));
    token_assert_consume(now, ";");
    return node;
}

// <expr> = <assign>
static Node *
parse_expr(Token **now)
{
    return parse_assign(now);
}

// <assign> = <cmp> ("=" <assign>)?
static Node *
parse_assign(Token **now)
{
    Token *tok = *now;
    Node *node = parse_cmp(&tok);

    if (token_consume(&tok, "="))
        node = new_binary(NT_ASSIGN, node, parse_assign(&tok));

    *now = tok;
    return node;
}

// <cmp> = <add> ("==" <add> | "!=" <add> | "<" <add> | "<=" <add> | ">" <add> | ">=" <add>)?
static Node *
parse_cmp(Token **now)
{
    Token *tok = *now;
    Node *node = parse_add(&tok);

    if (token_consume(&tok, "=="))
        node = new_binary(NT_EQ, node, parse_add(&tok));
    else if (token_consume(&tok, "!="))
        node = new_binary(NT_NE, node, parse_add(&tok));
    else if (token_consume(&tok, "<"))
        node = new_binary(NT_LT, node, parse_add(&tok));
    else if (token_consume(&tok, "<="))
        node = new_binary(NT_LE, node, parse_add(&tok));
    else if (token_consume(&tok, ">"))
        node = new_binary(NT_LT, parse_add(&tok), node);
    else if (token_consume(&tok, ">="))
        node = new_binary(NT_LE, parse_add(&tok), node);

    *now = tok;
    return node;
}

// <add> = <mul> ("+" <mul> | "-" <mul>)*
static Node *
parse_add(Token **now)
{
    Token *tok = *now;
    Node *node = parse_mul(&tok);

    for (;;)
    {
        if (token_eq(tok, "+"))
        {
            tok = tok->next;
            node = new_binary(NT_ADD, node, parse_mul(&tok));
            continue;
        }
        if (token_eq(tok, "-"))
        {
            tok = tok->next;
            node = new_binary(NT_SUB, node, parse_mul(&tok));
            continue;
        }
        break;
    }

    *now = tok;
    return node;
}

// <mul> = <unary> ("*" <unary> | "/" <unary>)*
static Node *
parse_mul(Token **now)
{
    Token *tok = *now;
    Node *node = parse_unary(&tok);

    for (;;)
    {
        if (token_eq(tok, "*"))
        {
            tok = tok->next;
            node = new_binary(NT_MUL, node, parse_unary(&tok));
            continue;
        }
        if (token_eq(tok, "/"))
        {
            tok = tok->next;
            node = new_binary(NT_DIV, node, parse_unary(&tok));
            continue;
        }
        break;
    }

    *now = tok;
    return node;
}

// <unary> = ("+" | "-") <unary>
//         | <primary>
static Node *
parse_unary(Token **now)
{
    Token *tok = *now;
    Node *node;

    if (token_consume(&tok, "+"))
        node = parse_unary(&tok);
    else if (token_consume(&tok, "-"))
        node = new_unary(NT_NEG, parse_unary(&tok));
    else
        node = parse_primary(&tok);

    *now = tok;
    return node;
}

// <primary> = "(" <expr> ")"
//           | <ident> ("(" ")")?
//           | <num>
static Node *
parse_primary(Token **now)
{
    Token *tok = *now;
    Node *node;

    // expression
    if (token_eq(tok, "("))
    {
        tok = tok->next;
        node = parse_expr(&tok);
        token_assert(tok, ")");
        *now = tok->next;
        return node;
    }
    if (tok->type == TT_IDENT)
    {
        // funciton call
        if (token_eq(tok->next, "("))
        {
            node = new_node(NT_FN_CALL);
            node->fn_name = strndup(tok->pos, tok->len);
            *now = tok->next->next;
            token_assert_consume(now, ")");
            return node;
        }
        // variable
        Obj *local = find_local(tok->pos, tok->len);
        if (!local)
            die("identifier %.*s not declared", tok->len, tok->pos);
        *now = tok->next;
        return node_var(local);
    }
    if (tok->type == TT_NUM)
    {
        *now = tok->next;
        return node_num(tok);
    }

    die("bad primary from token %d", tok->type);
}
