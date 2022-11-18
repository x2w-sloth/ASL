#include <string.h>
#include "aslc.h"

static Node *new_node(NodeType type);
static Node *new_binary(NodeType type, Node *lch, Node *rch);
static Node *node_num(Token *tok);
static Node *parse_expr(Token **now);
static Node *parse_mul(Token **now);
static Node *parse_primary(Token **now);

Node *
parse(Token *tok)
{
    // parse program as single numeric expression
    return parse_expr(&tok);
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
node_num(Token *tok)
{
    Node *node = new_node(NT_NUM);
    node->ival = tok->ival;

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

// <mul> = <primary> ("*" <primary> | "/" <primary>)*
static Node *
parse_mul(Token **now)
{
    Token *tok = *now;
    Node *node = parse_primary(&tok);

    for (;;)
    {
        if (token_eq(tok, "*"))
        {
            tok = tok->next;
            node = new_binary(NT_MUL, node, parse_primary(&tok));
            continue;
        }
        if (token_eq(tok, "/"))
        {
            tok = tok->next;
            node = new_binary(NT_DIV, node, parse_primary(&tok));
            continue;
        }
        break;
    }

    *now = tok;
    return node;
}

// <primary> = <num>
static Node *
parse_primary(Token **now)
{
    Token *tok = *now;
    Node *node;

    if (tok->type != TT_NUM)
        die("bad primary from token %d", tok->type);

    node = node_num(tok);
    *now = tok->next;
    return node;
}
