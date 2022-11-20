#include <string.h>
#include "aslc.h"

static Node *new_node(NodeType type);
static Node *new_binary(NodeType type, Node *lch, Node *rch);
static Node *new_unary(NodeType type, Node *lch);
static Node *node_num(Token *tok);
static Node *parse_stmt(Token **now);
static Node *parse_block_stmt(Token **now);
static Node *parse_expr_stmt(Token **now);
static Node *parse_expr(Token **now);
static Node *parse_mul(Token **now);
static Node *parse_unary(Token **now);
static Node *parse_primary(Token **now);

Node *
parse(Token *tok)
{
    Node dummy = {};
    Node *node = &dummy;

    // parse program as one or more statements
    while (tok->type != TT_END)
        node = node->next = parse_stmt(&tok);

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
node_num(Token *tok)
{
    Node *node = new_node(NT_NUM);
    node->ival = tok->ival;

    return node;
}

// <stmt> = "{" <block_stmt>
//        | <expr_stmt>
static Node *
parse_stmt(Token **now)
{
    if (token_consume(now, "{"))
        return parse_block_stmt(now);

    return parse_expr_stmt(now);
}

// <block_stmt> = (<stmt>)* "}"
static Node *
parse_block_stmt(Token **now)
{
    Token *tok = *now;
    Node dummy = {};
    Node *node = &dummy;

    while (!token_eq(tok, "}"))
        node = node->next = parse_stmt(&tok);

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
    token_assert(*now, ";");
    token_consume(now, ";");
    return node;
}

// <expr> = <mul> ("+" <mul> | "-" <mul>)*
static Node *
parse_expr(Token **now)
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
//           | <num>
static Node *
parse_primary(Token **now)
{
    Token *tok = *now;
    Node *node;

    if (token_eq(tok, "("))
    {
        tok = tok->next;
        node = parse_expr(&tok);
        token_assert(tok, ")");
    }
    else if (tok->type == TT_NUM)
        node = node_num(tok);
    else
        die("bad primary from token %d", tok->type);

    *now = tok->next;
    return node;
}
