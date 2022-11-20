#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "aslc.h"

static Token *new_token(TokenType type);
static Token *read_num(const char *pos);
static Token *read_punc(const char *pos);
static Token *read_ident(const char *pos);
static bool is_keyword(const Token *tok);
static bool is_ident1(const char c);
static bool is_ident2(const char c);

Token *
tokenize(const char *c)
{
    Token dummy;
    Token *tok = &dummy, *next;
    size_t len;

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
            tok = tok->next = read_punc(c);
            c += tok->len;
            continue;
        }
        if ((next = read_ident(c)))
        {
            tok = tok->next = next;
            if (is_keyword(tok))
                tok->type = TT_KEYWORD;
            c += tok->len;
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

bool
token_consume(Token **tok, const char *str)
{
    if (token_eq(*tok, str))
    {
        *tok = (*tok)->next;
        return true;
    }
    return false;;
}

void
token_assert_consume(Token **tok, const char *str)
{
    token_assert(*tok, str);
    token_consume(tok, str);
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

static Token *
read_punc(const char *pos)
{
    static const char *puncs[] = {
        "<<=", ">>=", "==", "!=", "<=", ">=", "->",
        "+=", "-=", "*=", "/=", "%=", "++", "--"
    };
    size_t len = 1;
    int i;

    if (!ispunct(*pos))
        return NULL;

    for (i = 0; i < COUNT(puncs); i++)
        if (!strncmp(pos, puncs[i], strlen(puncs[i])))
        {
            len = strlen(puncs[i]);;
            break;
        }

    Token *tok = new_token(TT_PUNC);
    tok->len = len;
    tok->pos = pos;
    return tok;
}

static Token *
read_ident(const char *pos)
{
    const char *c = pos;

    if (!is_ident1(*c++))
        return NULL;

    while (is_ident2(*c))
        ++c;

    Token *tok = new_token(TT_IDENT);
    tok->len = c - pos;
    tok->pos = pos;
    return tok;
}

static bool
is_keyword(const Token *tok)
{
    static const char *keywords[] = { "return" };

    for (int i = 0; i < COUNT(keywords); i++)
        if (token_eq(tok, keywords[i]))
            return true;
    return false;
}

static bool
is_ident1(const char c)
{
    return (c == '_') || ('A'<=c && c<='Z') || ('a'<=c && c<='z');
}

static bool
is_ident2(const char c)
{
    return is_ident1(c) || ('0'<=c && c <='9');
}
