#include <string.h>
#include "aslc.h"

#define is_i64(T)       is_type_int((T), 64)
#define is_ptr(T)       ((T)->type == DT_PTR)

static Node *new_node(NodeType type);
static Node *new_binary(NodeType type, Node *lch, Node *rch);
static Node *new_unary(NodeType type, Node *lch);
static Node *node_var(const char *name);
static Node *node_num(int64_t ival);
static Obj *new_obj(ObjType type);
static Obj *obj_local(Type *dt, const char *name);
static Obj *obj_global(Type *dt, const char *name);
static Obj *obj_fn(const char *name);
static void obj_params(Type *param);
static Obj *find_local(const char *name, size_t len);
static Obj *find_global(const char *name, size_t len);
static Obj *find_fn(const char *name, size_t len);
static Type *new_type(DataType type);
static Type *copy_type(const Type *dt);
static Type *type_pointer(Type *base);
static bool is_type_int();
static void parse_globals(Token **now);
static void parse_fn(Token **now);
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
static char *get_ident_str(Token *tok);
static int64_t get_num_ival(Token *tok);
static void semantics(Node **node);
static void sem_add(Node **node);
static void sem_sub(Node **node);
static void sem_mul(Node **node);
static void sem_div(Node **node);

static Obj *locals;
static Obj *globals;
static const Type type_none = { .type = DT_NONE };
static const Type type_i64  = { .type = DT_INT, .size = 8, .bits = 64 };

Obj *
parse(Token *tok)
{
    globals = NULL;

    // construct AST and allocate objects
    while (tok->type != TT_END)
        (token_eq(tok, "fn") ? parse_fn : parse_globals)(&tok);

    // check type semantics and mutate AST as necessary
    for (Obj *obj = globals; obj; obj = obj->next)
        if (obj->type == OT_FN)
            semantics(&obj->body);

    return globals;
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
node_var(const char *name)
{
    Node *node = new_node(NT_VAR);

    node->var_name = name;

    return node;
}

static Node *
node_num(int64_t ival)
{
    Node *node = new_node(NT_NUM);
    node->ival = ival;

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
obj_global(Type *dt, const char *name)
{
    Obj *obj = new_obj(OT_GLOBAL);

    obj->dt = dt;
    obj->name = name;
    obj->next = globals;
    globals = obj;

    return obj;
}

static Obj *
obj_fn(const char *name)
{
    Obj *obj = new_obj(OT_FN);

    obj->dt = new_type(DT_FN);
    obj->name = name;
    obj->next = globals;
    globals = obj;

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

static Obj *
find_global(const char *name, size_t len)
{
    for (Obj *obj = globals; obj; obj = obj->next)
        if (strlen(obj->name) == len && !strncmp(name, obj->name, len) && obj->type == OT_GLOBAL)
            return obj;
    return NULL;
}

static Obj *
find_fn(const char *name, size_t len)
{
    for (Obj *obj = globals; obj; obj = obj->next)
        if (strlen(obj->name) == len && !strncmp(name, obj->name, len) && obj->type == OT_FN)
            return obj;
    return NULL;
}

static Type *
new_type(DataType type)
{
    Type *dt = xmalloc(sizeof(Type));
    memset(dt, 0, sizeof(Type));
    dt->type = type;

    return dt;
}

static Type *
copy_type(const Type *dt)
{
    Type *ndt = xmalloc(sizeof(Type));
    *ndt = *dt;

    return ndt;
}

static Type *
type_pointer(Type *base)
{
    Type *dt = new_type(DT_PTR);
    dt->base = base;

    return dt;
}

static bool
is_type_int(const Type *dt, int bits)
{
    return dt->type == DT_INT && dt->bits == bits;
}

// <globals> = <declspec> <ident> ("," <ident>)* ";"
static void
parse_globals(Token **now)
{
    Token *tok = *now;
    Type *dt = parse_declspec(&tok);

    if (token_eq(tok, ";"))
        die("expected global variable identifier");

    while (!token_eq(tok, ";"))
    {
        token_consume(&tok, ",");
        obj_global(dt, get_ident_str(tok));
        tok = tok->next;
    }

    *now = tok->next;
    return;
}

// <fn> = "fn" <ident> "(" (<fn_params>)? ")" (<declspec>)? "{" <block_stmt>
// <fn_params> = <declspec> <ident>("," <declspec> <ident>)*
static void
parse_fn(Token **now)
{
    Token *tok = *now;
    Type dummy = {};
    Type *param_dt = &dummy;

    token_assert_consume(&tok, "fn");

    Obj *fn = obj_fn(get_ident_str(tok));
    tok = tok->next;

    // parse fn parameters
    token_assert_consume(&tok, "(");
    while (!token_eq(tok, ")"))
    {
        token_consume(&tok, ",");
        param_dt = param_dt->next = parse_declspec(&tok);
        param_dt->name = get_ident_str(tok);
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
    else
        fn->dt->ret = copy_type(&type_none);
    token_assert_consume(&tok, "{");

    // parse fn body
    fn->body = parse_block_stmt(&tok);
    fn->locals = locals;

    *now = tok;
    return;
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

        Obj *local = obj_local(dt, get_ident_str(tok));

        tok = tok->next;
        if (!token_consume(&tok, "="))
            continue;

        // assign value on declaration
        Node *lch = node_var(local->name);
        Node *rch = parse_assign(&tok);
        lch->var = local;
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
    Obj *var;

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
        // function call
        if (token_eq(tok->next, "("))
        {
            node = parse_fn_call(&tok);
            *now = tok;
            return node;
        }
        // variable
        // if we can't find as local var, find as global var during semantics
        node = node_var(get_ident_str(tok));
        node->var = find_local(node->var_name, strlen(node->var_name));
        *now = tok->next;
        return node;
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
    node->fn_name = get_ident_str(iden);
    node->fn_args = dummy.next;
    return node;
}

static char *
get_ident_str(Token *tok)
{
    if (tok->type != TT_IDENT)
        die("expected identifier token");
    return strndup(tok->pos, tok->len);
}

static int64_t
get_num_ival(Token *tok)
{
    if (tok->type != TT_NUM)
        die("expected numeric token");
    return tok->ival;
}

static void
semantics(Node **node_)
{
    Node *node = *node_;

    if (!node || node->dt)
        return;

    semantics(&node->lch);
    semantics(&node->rch);
    semantics(&node->init);
    semantics(&node->cond);
    semantics(&node->iter);
    semantics(&node->br_if);
    semantics(&node->br_else);
    for (Node **stmt = &node->block; *stmt; stmt = &(*stmt)->next)
        semantics(stmt);

    // annotate data type for statement nodes and below
    Obj *fn, *var;
    switch (node->type)
    {
        case NT_RET_STMT:
        case NT_EXPR_STMT:
        case NT_IF_STMT:
        case NT_FOR_STMT:
        case NT_BLOCK_STMT:
            node->dt = copy_type(&type_none);
            return;
        case NT_ADD:
            sem_add(node_);
            node->dt = node->lch->dt;
            return;
        case NT_SUB:
            sem_sub(node_);
            node->dt = node->lch->dt;
            return;
        case NT_MUL:
            sem_mul(node_);
            node->dt = node->lch->dt;
            return;
        case NT_DIV:
            sem_div(node_);
            node->dt = node->lch->dt;
            return;
        case NT_NEG:
        case NT_ASSIGN:
            node->dt = node->lch->dt;
            return;
        case NT_DEREF:
            if (node->lch->dt->type != DT_PTR)
                die("attempt to deref a non-pointer");
            node->dt = node->lch->dt->base;
            return;
        case NT_ADDR:
            node->dt = type_pointer(node->lch->dt);
            return;
        case NT_VAR:
            if (node->var) // local variable
            {
                node->dt = node->var->dt;
                return;
            }
            var = find_global(node->var_name, strlen(node->var_name));
            if (!var)
                die("identifier %s not declared", node->var_name);
            node->var = var;
            node->dt = var->dt;
            return;
        case NT_EQ:
        case NT_NE:
        case NT_LT:
        case NT_LE:
        case NT_NUM:
            node->dt = copy_type(&type_i64);
            return;
        case NT_FN_CALL:
            fn = find_fn(node->fn_name, strlen(node->fn_name));
            if (!fn || fn->type != OT_FN)
                die("function %s not found", node->fn_name);
            node->dt = fn->dt->ret;
            return;
        default:
            die("can not annotate data type for node %d", node->type);
    }
}

static void
sem_add(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch, *rch = node->rch;

    // i64 + i64
    if (is_i64(lch->dt) && is_i64(rch->dt))
        return;
    // canonicalize i64 + ptr
    if (is_i64(lch->dt) && is_ptr(rch->dt))
    {
        Node *tmp = node->lch;
        node->lch = node->rch;
        node->rch = tmp;
    }
    // ptr + i64
    if (is_ptr(lch->dt) && is_i64(rch->dt))
    {
        node->rch = new_binary(NT_MUL, rch, node_num(8));
        node->rch->dt = copy_type(&type_i64); //
        return;
    }

    die("bad add between %d and %d", lch->dt->type, rch->dt->type);
}

static void
sem_sub(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch, *rch = node->rch;

    // i64 - i64
    if (is_i64(lch->dt) && is_i64(rch->dt))
        return;
    // ptr - i64
    if (is_ptr(lch->dt) && is_i64(rch->dt))
    {
        node->rch = new_binary(NT_MUL, rch, node_num(8));
        node->rch->dt = copy_type(&type_i64); //
        return;
    }
    // ptr - ptr
    if (is_ptr(lch->dt) && is_ptr(rch->dt))
    {
        *node_ = new_binary(NT_DIV, node, node_num(8));
        (*node_)->dt = copy_type(&type_i64);
        return;
    }

    die("bad sub between %d and %d", lch->dt->type, rch->dt->type);
}

static void
sem_mul(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch, *rch = node->rch;

    if (is_i64(lch->dt) && is_i64(rch->dt))
        return;

    die("bad mul between %d and %d", lch->dt->type, rch->dt->type);
}

static void
sem_div(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch, *rch = node->rch;

    if (is_i64(lch->dt) && is_i64(rch->dt))
        return;

    die("bad div between %d and %d", lch->dt->type, rch->dt->type);
}
