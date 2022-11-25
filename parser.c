#include <string.h>
#include "aslc.h"

static Node *new_node(NodeType type);
static Node *new_binary(NodeType type, Node *lch, Node *rch);
static Node *new_unary(NodeType type, Node *lch);
static Node *node_var(Obj *var);
static Node *node_num(int64_t ival);
static Node *node_add(Node *lch, Node *rch);
static Node *node_sub(Node *lch, Node *rch);
static Node *node_mul(Node *lch, Node *rch);
static Node *node_div(Node *lch, Node *rch);
static Obj *new_obj(ObjType type);
static Obj *obj_local(Type *dt, const char *name);
static Obj *obj_fn(const char *name);
static void obj_params(Type *param);
static Obj *find_local(const char *name, size_t len);
static Obj *parse_fn(Token **now);
static Node *parse_decl(Token **now);
static Type *parse_declspec(Token **now);
static Node *parse_declarator(Token **now, Type *dt);
static Node *parse_stmt(Token **now);
static Node *parse_block_stmt(Token **now);
static Node *parse_expr_stmt(Token **now);
static Node *parse_if_stmt(Token **now);
static Node *parse_for_stmt(Token **now);
static Node *parse_expr(Token **now);
static Node *parse_assign(Token **now);
static Node *parse_cmp(Token **now);
static Node *parse_add(Token **now);
static Node *parse_mul(Token **now);
static Node *parse_unary(Token **now);
static Node *parse_primary(Token **now);
static Node *parse_fn_call(Token **now);

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
node_num(int64_t ival)
{
    Node *node = new_node(NT_NUM);
    node->ival = ival;

    return node;
}

static Node *
node_add(Node *lch, Node *rch)
{
    add_dt(lch);
    add_dt(rch);

    // i64 + i64
    if (is_i64(lch->dt) && is_i64(rch->dt))
        return new_binary(NT_ADD, lch, rch);
    // canonicalize i64 + ptr
    if (is_i64(lch->dt) && is_ptr(rch->dt))
    {
        Node *tmp = lch;
        lch = rch;
        rch = tmp;
    }
    // ptr + i64
    if (is_ptr(lch->dt) && is_i64(rch->dt))
        return new_binary(NT_ADD, lch, new_binary(NT_MUL, rch, node_num(8)));

    die("bad add between %d and %d", lch->dt->type, rch->dt->type);
}

static Node *
node_sub(Node *lch, Node *rch)
{
    add_dt(lch);
    add_dt(rch);

    // i64 - i64
    if (is_i64(lch->dt) && is_i64(rch->dt))
        return new_binary(NT_SUB, lch, rch);
    // ptr - i64
    if (is_ptr(lch->dt) && is_i64(rch->dt))
        return new_binary(NT_SUB, lch, new_binary(NT_MUL, rch, node_num(8)));
    // ptr - ptr
    if (is_ptr(lch->dt) && is_ptr(rch->dt))
    {
        Node *node = new_binary(NT_DIV, new_binary(NT_SUB, lch, rch), node_num(8));
        node->dt = &type_i64;
        return node;
    }

    die("bad sub between %d and %d", lch->dt->type, rch->dt->type);
}

static Node *
node_mul(Node *lch, Node *rch)
{
    add_dt(lch);
    add_dt(rch);

    if (is_i64(lch->dt) && is_i64(rch->dt))
        return new_binary(NT_MUL, lch, rch);

    die("bad mul between %d and %d", lch->dt->type, rch->dt->type);
}

static Node *
node_div(Node *lch, Node *rch)
{
    add_dt(lch);
    add_dt(rch);

    if (is_i64(lch->dt) && is_i64(rch->dt))
        return new_binary(NT_DIV, lch, rch);

    die("bad div between %d and %d", lch->dt->type, rch->dt->type);
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
obj_local(Type *dt, const char *name)
{
    Obj *obj = new_obj(OT_LOCAL);

    obj->dt = dt;
    obj->name = name;
    obj->next = locals;
    locals = obj;

    return obj;
}

static Obj *
obj_fn(const char *name)
{
    Obj *obj = new_obj(OT_FN);

    obj->dt = new_type(DT_FN);
    obj->name = name;

    return obj;
}

static void
obj_params(Type *param)
{
    if (param) {
        obj_params(param->next);
        obj_local(param, param->name);
    }
}

static Obj *
find_local(const char *name, size_t len)
{
    for (Obj *obj = locals; obj; obj = obj->next)
        if (strlen(obj->name) == len && !strncmp(name, obj->name, len))
            return obj;
    return NULL;
}

// <fn> = "fn" <ident> "(" (<fn_params>)? ")" (<declspec>)? "{" <block_stmt>
// <fn_params> = <declspec> <ident>("," <declspec> <ident>)*
static Obj *
parse_fn(Token **now)
{
    Token *tok = *now;
    Type dummy = {};
    Type *param_dt = &dummy;

    token_assert_consume(&tok, "fn");
    if (tok->type != TT_IDENT)
        die("expected function name identifier");

    Obj *fn = obj_fn(strndup(tok->pos, tok->len));
    tok = tok->next;

    // parse fn parameters
    token_assert_consume(&tok, "(");
    while (!token_eq(tok, ")"))
    {
        token_consume(&tok, ",");
        param_dt = param_dt->next = parse_declspec(&tok);
        if (tok->type != TT_IDENT)
            die("expected parameter name identifier");
        param_dt->name = strndup(tok->pos, tok->len);
        tok = tok->next;
    }
    token_assert_consume(&tok, ")");

    // allocate fn parameters as locals
    locals = NULL;
    obj_params(dummy.next);
    fn->dt->params = dummy.next;
    fn->params = locals;
    
    // parse optional fn return type
    if (!token_eq(tok, "{"))
        fn->dt->ret = parse_declspec(&tok);
    token_assert_consume(&tok, "{");

    // parse fn body
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

    Type *decl_dt = parse_declspec(&tok);

    Node *node = new_node(NT_BLOCK_STMT);
    node->block = parse_declarator(&tok, decl_dt);

    *now = tok;
    return node;
}

// <declspec> = "i64" ("*")*
static Type *
parse_declspec(Token **now)
{
    Token *tok = *now;

    token_assert_consume(&tok, "i64");
    Type *dt = copy_type(&type_i64);

    while (token_consume(&tok, "*"))
        dt = type_pointer(dt);

    *now = tok;
    return dt;
}

// <declarator> = <ident> ("=" <expr>)? ("," <ident> ("=" <expr>)?)* ";"
static Node *
parse_declarator(Token **now, Type *dt)
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

        Obj *local = obj_local(dt, strndup(tok->pos, tok->len));

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
//        | <if_stmt>
//        | <for_stmt>
//        | <expr_stmt>
static Node *
parse_stmt(Token **now)
{
    if (token_consume(now, "{"))
        return parse_block_stmt(now);
    if (token_consume(now, "return"))
        return new_unary(NT_RET_STMT, parse_expr_stmt(now));
    if (token_eq(*now, "if"))
        return parse_if_stmt(now);
    if (token_eq(*now, "for"))
        return parse_for_stmt(now);

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
        else {
            node = node->next = parse_stmt(&tok);
            add_dt(node);
        }
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

// <if_stmt> = "if" <expr> "{" <block_stmt> ("else" ("{" <block_stmt> | <if_stmt>))?
static Node *
parse_if_stmt(Token **now)
{
    Token *tok = *now;
    Node *node = new_node(NT_IF_STMT);

    token_assert_consume(&tok, "if");
    node->cond = parse_expr(&tok);
    token_assert_consume(&tok, "{");
    node->br_if = parse_block_stmt(&tok);

    // optional else branch
    if (token_consume(&tok, "else"))
    {
        if (token_eq(tok, "if"))
            node->br_else = parse_if_stmt(&tok);
        else
        {
            token_assert_consume(&tok, "{");
            node->br_else = parse_block_stmt(&tok);
        }
    }

    *now = tok;
    return node;
}

// <for_stmt> = "for" (<expr> (";" <expr_stmt> (<expr>)?)?)? "{" <block_stmt>
static Node *
parse_for_stmt(Token **now)
{
    Token *tok = *now;
    Node *node = new_node(NT_FOR_STMT);

    token_assert_consume(&tok, "for");
    if (token_appears_before(tok, ";", "{")) // classic for loop
    {
        node->init = parse_expr_stmt(&tok);
        if (!token_eq(tok, ";"))
            node->cond = parse_expr(&tok);
        token_assert_consume(&tok, ";");
        if (!token_eq(tok, "{"))
            node->iter = parse_expr(&tok);
    }
    else if (!token_eq(tok, "{")) // while loop
        node->cond = parse_expr(&tok);

    // loop body
    token_assert_consume(&tok, "{");
    node->block = parse_block_stmt(&tok);

    *now = tok;
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
            node = node_add(node, parse_mul(&tok));
            continue;
        }
        if (token_eq(tok, "-"))
        {
            tok = tok->next;
            node = node_sub(node, parse_mul(&tok));
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
            node = node_mul(node, parse_unary(&tok));
            continue;
        }
        if (token_eq(tok, "/"))
        {
            tok = tok->next;
            node = node_div(node, parse_unary(&tok));
            continue;
        }
        break;
    }

    *now = tok;
    return node;
}

// <unary> = ("+" | "-" | "*" | "&") <unary>
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
    else if (token_consume(&tok, "*"))
        node = new_unary(NT_DEREF, parse_unary(&tok));
    else if (token_consume(&tok, "&"))
        node = new_unary(NT_ADDR, parse_unary(&tok));
    else
        node = parse_primary(&tok);

    *now = tok;
    return node;
}

// <primary> = "(" <expr> ")"
//           | <fn_call>
//           | <ident>
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
            node = parse_fn_call(&tok);
            *now = tok;
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
        return node_num(tok->ival);
    }

    die("bad primary from token %d", tok->type);
}

// <fn_call> = <ident> "(" (<assign> ("," <assign>)*)? ")"
static Node *
parse_fn_call(Token **now)
{
    Token *iden = *now;
    Token *tok = (*now)->next->next;
    Node dummy = {};
    Node *node = &dummy;

    while (!token_eq(tok, ")"))
    {
        token_consume(&tok, ",");
        node = node->next = parse_assign(&tok);
    }

    token_assert_consume(&tok, ")");
    *now = tok;

    node = new_node(NT_FN_CALL);
    node->fn_name = strndup(iden->pos, iden->len);
    node->fn_args = dummy.next;
    return node;
}
