#include <string.h>
#include "aslc.h"

static Node *new_node(NodeType type);
static Node *node_num(Token *tok);
static Node *parse_expr(Token **now);

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
node_num(Token *tok)
{
    Node *node = new_node(NT_NUM);
    node->ival = tok->ival;

    return node;
}

// <expr> = <num>
static Node *
parse_expr(Token **now)
{
    Token *tok = *now;
    Node *node;

    if (tok->type != TT_NUM)
        die("bad expression");

    node = node_num(tok);
    *now = tok->next;
    return node;
}
