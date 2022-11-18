#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "aslc.h"

static Token *new_token(TokenType type);
static Token *read_num(const char *pos);

Token *
tokenize(const char *c)
{
    Token dummy;
    Token *tok = &dummy;

    while (*c != '\0')
    {
        if (isspace(*c))
        {
            ++c;
            continue;
        }
        if (isdigit(*c))
        {
            tok = tok->next = read_num(c);
            c += tok->len;
            continue;
        }
    }

    return dummy.next;
}

static Token *
new_token(TokenType type)
{
    Token *tok = xmalloc(sizeof(Token));
    memset(tok, 0, sizeof(Token));

    tok->type = type;

    return tok;
}

static Token *
read_num(const char *pos)
{
    Token *tok = new_token(TT_NUM);

    tok->pos = pos;
    tok->ival = atoi(pos);
    while (isdigit(*pos))
        pos++;
    tok->len = pos - tok->pos;

    return tok;
}
