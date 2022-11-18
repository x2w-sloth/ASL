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
        if (ispunct(*c))
        {
            tok = tok->next = new_token(TT_PUNC);
            tok->len = 1;
            tok->pos = c++;
            continue;
        }
    }

    tok->next = new_token(TT_END);
    return dummy.next;
}

bool
token_eq(const Token *tok, const char *str)
{
    return tok->len == strlen(str) && !memcmp(tok->pos, str, tok->len);
}

void
token_assert(const Token *tok, const char *str)
{
    if (!token_eq(tok, str))
        die("token expected: %s, got %.*s", str, tok->len, tok->pos);
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
