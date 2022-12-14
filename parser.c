#include <string.h>
#include "aslc.h"

#define is_i8(T)        (is_int(T) && (T)->size == 1)
#define is_i32(T)       (is_int(T) && (T)->size == 4)
#define is_i64(T)       (is_int(T) && (T)->size == 8)
#define is_int(T)       ((T)->type == DT_INT)
#define is_ptr(T)       ((T)->type == DT_PTR)
#define is_arr(T)       ((T)->type == DT_ARR)

static Node *new_node(NodeType type);
static Node *new_binary(NodeType type, Node *lch, Node *rch);
static Node *new_unary(NodeType type, Node *lch);
static Node *node_var(const char *name);
static Node *node_num(int64_t ival);
static Obj *new_obj(ObjType type);
static Obj *obj_local(Type *dt, const char *name);
static Obj *obj_global(Type *dt, const char *name);
static Obj *obj_member(Type *dt, const char *name);
static Obj *obj_fn(const char *name);
static void obj_params(Type *param);
static Path *new_path(const char *name);
static Scope *new_scope(const char *name);
static void scope_enter(Scope *sc);
static void scope_leave(void);
static void scope_semantics(Scope *sc);
static Scope *scope_lookup(const Path *path);
static BlockScope *new_block_scope();
static void block_scope_enter();
static void block_scope_leave();
static Obj *find_local(const char *name, size_t len);
static Obj *find_global(const char *name, size_t len, const Path *path);
static Obj *find_fn(const char *name, size_t len, const Path *path);
static Type *find_struct(const char *name, size_t len, const Path *path);
static Type *find_user_type(const Type *ut);
static void deduce_user_type(Type **dt);
static Type *new_type(DataType type);
static Type *copy_type(const Type *dt);
static Type *type_pointer(Type *base);
static Type *type_array(Type *base, int len);
static bool is_type_name(Token *tok);
static bool is_user_decl(Token *tok);
static bool is_fn_call(Token *tok);
static bool is_scope_path(Token *tok);
static void parse_scope(Token **now);
static void parse_struct(Token **now);
static void parse_globals(Token **now);
static void parse_fn(Token **now);
static Node *parse_decl(Token **now);
static Type *parse_declspec(Token **now);
static Node *parse_declarator(Token **now, Type *dt, ObjType ot);
static Type *parse_array(Token **now, Type *type);
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
static Node *parse_postfix(Token **now);
static Node *parse_index(Token **now, Node *primary);
static Node *parse_member(Token **now, Node *primary);
static Node *parse_primary(Token **now);
static Node *parse_fn_call(Token **now);
static Node *parse_var(Token **now);
static Path *parse_scope_path(Token **now);
static char *get_ident_str(Token *tok);
static int64_t get_num_ival(Token *tok);
static void sem(Node **node);
static void sem_add(Node **node);
static void sem_sub(Node **node);
static void sem_mul(Node **node);
static void sem_div(Node **node);
static void sem_mod(Node **node);
static void sem_index(Node **node);
static void sem_member(Node **node);
static void sem_deref(Node **node);
static void sem_var(Node **node);

static Obj *locals;
static Obj *members;
static BlockScope *bsc_now;
static Scope *sc_now, sc_root;
static const Type type_user_def = { .type = DT_USER_DEF };
static const Type type_none = { .type = DT_NONE };
static const Type type_bool = { .type = DT_BOOL, .size = 1 };
static const Type type_i8   = { .type = DT_INT, .size = 1 };
static const Type type_i16  = { .type = DT_INT, .size = 2 };
static const Type type_i32  = { .type = DT_INT, .size = 4 };
static const Type type_i64  = { .type = DT_INT, .size = 8 };

Scope *
parse(Token *tok)
{
    scope_enter(&sc_root);

    // First pass traverses tokens
    // construct AST and allocate objects along the way
    while (tok->type != TT_END)
    {
        if (token_eq(tok, "scope"))
            parse_scope(&tok);
        else if (token_eq(tok, "struct"))
            parse_struct(&tok);
        else if (token_eq(tok, "fn"))
            parse_fn(&tok);
        else
            parse_globals(&tok);
    }

    // Second pass traverses AST
    // check type semantics for each node and mutate AST as necessary
    scope_semantics(&sc_root);

    if (sc_now != &sc_root)
        die("scope depth error");
    scope_leave();

    return &sc_root;
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

static Node *
node_bval(int val)
{
    if (val != 0 && val != 1)
        die("bad boolean value");

    Node *node = new_node(NT_BVAL);
    node->ival = val;
    
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

    // local vars live in blockscopes
    obj->dt = dt;
    obj->name = name;
    obj->next = locals;
    locals = obj;

    obj->bnext = bsc_now->locals;
    bsc_now->locals = obj;

    return obj;
}

static Obj *
obj_global(Type *dt, const char *name)
{
    Obj *obj = new_obj(OT_GLOBAL);

    // global vars live in named scopes, or the root scope
    obj->dt = dt;
    obj->name = name;
    obj->next = sc_now->globals;
    sc_now->globals = obj;

    return obj;
}

static Obj *
obj_member(Type *dt, const char *name)
{
    Obj *obj = new_obj(OT_MEMBER);

    obj->dt = dt;
    obj->name = name;
    obj->next = members;
    members = obj;

    return obj;
}

static Obj *
obj_fn(const char *name)
{
    Obj *obj = new_obj(OT_FN);

    obj->dt = new_type(DT_FN);
    obj->scope = sc_now;
    obj->name = name;
    obj->next = sc_now->fns;
    sc_now->fns = obj;

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

static Path *
new_path(const char *name)
{
    Path *path = xmalloc(sizeof(Path));
    memset(path, 0, sizeof(Path));

    path->name = name;

    return path;
}

static Scope *
new_scope(const char *name)
{
    Scope *sc = xmalloc(sizeof(Scope));
    memset(sc, 0, sizeof(Scope));
    
    sc->name = name;
    sc->parent = sc_now;

    return sc;
}

static void
scope_enter(Scope *sc)
{
    if (sc_now)
    {
        sc->next = sc_now->children;
        sc_now->children = sc;
    }

    sc->parent = sc_now;
    sc_now = sc;
}

static void
scope_leave(void)
{
    sc_now = sc_now->parent;
}

static void
scope_semantics(Scope *sc)
{
    if (!sc)
        return;

    // calculate struct size and assign member offsets
    // if a struct has members of user defined types, deduce it now
    for (Type *st = sc->structs; st; st = st->next)
    {
        int offset = 0;
        for (Obj *mem = st->members; mem; mem = mem->next)
        {
            deduce_user_type(&mem->dt);
            mem->mem_off = offset;
            offset += mem->dt->size;
        }
        st->size = offset;
    }

    for (Obj *fn = sc->fns; fn; fn = fn->next)
    {
        // if a local variable is user defined type, deduce it now
        for (Obj *local = fn->locals; local; local = local->next)
            deduce_user_type(&local->dt);
        sem(&fn->body);
    }

    sc = sc->children;
    for (Scope *s = sc; s; s = s->next)
        scope_semantics(s);
}

static Scope *
scope_lookup(const Path *p)
{
    Scope *sc = &sc_root, *s;

    while (p)
    {
        for (s = sc->children; s; s = s->next)
            if (!strcmp(s->name, p->name))
            {
                p = p->next;
                sc = s;
                break;
            }
        if (!s)
            break;
    }
    return sc;
}

static BlockScope *
new_block_scope()
{
    BlockScope *bsc = xmalloc(sizeof(BlockScope));
    memset(bsc, 0, sizeof(BlockScope));

    return bsc;
}

static void
block_scope_enter()
{
    BlockScope *bsc = new_block_scope();

    bsc->next = bsc_now;
    bsc_now = bsc;
}

static void
block_scope_leave()
{
    bsc_now = bsc_now->next;
}

static Obj *
find_local(const char *name, size_t len)
{
    for (BlockScope *bsc = bsc_now; bsc; bsc = bsc->next)
        for (Obj *obj = bsc->locals; obj; obj = obj->bnext)
            if (strlen(obj->name) == len && !strncmp(name, obj->name, len))
                return obj;
    return NULL;
}

static Obj *
find_global(const char *name, size_t len, const Path *path)
{
    Scope *sc = scope_lookup(path);

    for (Obj *obj = sc->globals; obj; obj = obj->next)
        if (strlen(obj->name) == len && !strncmp(name, obj->name, len))
            return obj;

    return NULL;
}

static Obj *
find_fn(const char *name, size_t len, const Path *path)
{
    Scope *sc = scope_lookup(path);

    for (Obj *obj = sc->fns; obj; obj = obj->next)
        if (strlen(obj->name) == len && !strncmp(name, obj->name, len))
            return obj;
    return NULL;
}

static Type *
find_struct(const char *name, size_t len, const Path *path)
{
    Scope *sc = scope_lookup(path);

    for (Type *st = sc->structs; st; st = st->next)
        if (!strncmp(name, st->name, len))
            return st;
    return NULL;
}

static Type *
find_user_type(const Type *ut)
{
    Type *t;

    if (ut->type != DT_USER_DEF)
        die("not a user defined type");

    if ((t = find_struct(ut->name, strlen(ut->name), ut->path)))
        return t;

    die("user defined type %s not declared", ut->name);
}

static void
deduce_user_type(Type **dt_)
{
    Type *dt = *dt_;

    if (dt->type == DT_USER_DEF)
        *dt_ = find_user_type(dt);

    if (dt->type == DT_ARR && dt->base->type == DT_USER_DEF)
        *dt_ = type_array(find_user_type(dt->base), dt->arr_len);
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
    dt->size = 8;

    return dt;
}

static Type *
type_array(Type *base, int len)
{
    Type *dt = new_type(DT_ARR);
    dt->base = base;
    dt->size = base->size * len;
    dt->arr_len = len;

    return dt;
}

static bool
is_type_name(Token *tok)
{
    static const char *names[] = { "bool", "i8", "i16", "i32", "i64" };

    for (int i = 0; i < COUNT(names); i++)
        if (token_eq(tok, names[i]))
            return true;
    return false;
}

static bool
is_user_decl(Token *tok)
{
    while (is_scope_path(tok))
        tok = tok->next->next;
    return tok->type == TT_IDENT && tok->next->type == TT_IDENT;
}

static bool
is_fn_call(Token *tok)
{
    while (tok->type == TT_IDENT && token_eq(tok->next, ":"))
        tok = tok->next->next;
    return token_eq(tok->next, "(");
}

static bool
is_scope_path(Token *tok)
{
    return tok->type == TT_IDENT && token_eq(tok->next, ":");
}

// <scope> = "scope" <ident> "{" (<scope> | <fn> | <globals>)* "}"
static void
parse_scope(Token **now)
{
    Token *tok = *now;
    Scope *sc;
    
    token_assert_consume(&tok, "scope");
    sc = new_scope(get_ident_str(tok));
    tok = tok->next;
    token_assert_consume(&tok, "{");

    scope_enter(sc);

    while (!token_consume(&tok, "}"))
    {
        if (token_eq(tok, "scope"))
            parse_scope(&tok);
        else if (token_eq(tok, "struct"))
            parse_struct(&tok);
        else if (token_eq(tok, "fn"))
            parse_fn(&tok);
        else
            parse_globals(&tok);
    }

    scope_leave();

    *now = tok;
    return;
}

// <struct> = "struct" <ident> "{" (<declspec> <declarator>)* "}"
static void
parse_struct(Token **now)
{
    Token *tok = *now;
    Type *st, *mt;

    token_assert_consume(&tok, "struct");

    st = new_type(DT_STRUCT);
    st->name = get_ident_str(tok);
    tok = tok->next;

    members = NULL;

    token_assert_consume(&tok, "{");
    while (!token_eq(tok, "}"))
    {
        mt = parse_declspec(&tok);
        parse_declarator(&tok, mt, OT_MEMBER);
    }

    // register struct under current scope
    st->members = members;
    st->next = sc_now->structs;
    sc_now->structs = st;

    *now = tok->next;
}

// <globals> = <declspec> <declarator>
static void
parse_globals(Token **now)
{
    Token *tok = *now;
    Type *dt = parse_declspec(&tok);

    parse_declarator(&tok, dt, OT_GLOBAL);

    *now = tok;
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

    block_scope_enter();

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

    block_scope_leave();

    *now = tok;
    return;
}

// <decl> = <declspec> <declarator>
static Node *
parse_decl(Token **now)
{
    Token *tok = *now;
    Type *decl_dt = parse_declspec(&tok);

    // declare local variables in block scope
    Node *node = new_node(NT_BLOCK_STMT);
    node->block = parse_declarator(&tok, decl_dt, OT_LOCAL);

    *now = tok;
    return node;
}

// <declspec> = ("bool" | "i8" | "i16" | "i32" | "i64" | <user_decl>) ("*")*
static Type *
parse_declspec(Token **now)
{
    Token *tok = *now;
    Type *dt;

    if (token_consume(&tok, "bool"))
        dt = copy_type(&type_bool);
    else if (token_consume(&tok, "i8"))
        dt = copy_type(&type_i8);
    else if (token_consume(&tok, "i16"))
        dt = copy_type(&type_i16);
    else if (token_consume(&tok, "i32"))
        dt = copy_type(&type_i32);
    else if (token_consume(&tok, "i64"))
        dt = copy_type(&type_i64);
    else
    {
        // user declared type, validated during semantics
        dt = copy_type(&type_user_def);
        if (is_scope_path(tok))
            dt->path = parse_scope_path(&tok);
        dt->name = get_ident_str(tok);
        tok = tok->next;
    }

    while (token_consume(&tok, "*"))
        dt = type_pointer(dt);

    *now = tok;
    return dt;
}

// <declarator> = <ident> ("[" <array>)? ("=" <expr>)? ("," <ident> ("[" <array>)? ("=" <expr>)?)* ";"
static Node *
parse_declarator(Token **now, Type *dt, ObjType ot)
{
    Token *tok = *now;
    Node dummy = {};
    Node *node = &dummy;
    Obj *local;
    const char *name;

    if (tok->type != TT_IDENT)
        die("expected at least one identifier in declarator");

    while (!token_consume(&tok, ";"))
    {
        token_consume(&tok, ",");

        if (tok->type != TT_IDENT)
            die("expected identifier in declarator, got %d", tok->type);

        name = get_ident_str(tok);
        tok = tok->next;

        if (token_consume(&tok, "[")) // array type
            dt = parse_array(&tok, dt);

        if (ot == OT_LOCAL) // declare local variable
        {
            if (find_local(name, strlen(name)))
                die("local var %s already declared", name);

            local = obj_local(dt, name);

            if (!token_consume(&tok, "="))
                continue;

            // assign value on declaration
            Node *lch = node_var(local->name);
            Node *rch = parse_assign(&tok);
            lch->var = local;
            node = node->next = new_unary(NT_EXPR_STMT, new_binary(NT_ASSIGN, lch, rch));
        }
        else if (ot == OT_GLOBAL) // declare global variable
            obj_global(dt, name);
        else if (ot == OT_MEMBER) // declare struct member
            obj_member(dt, name);
        else
            die("can not declare object type %d", ot);
    }

    *now = tok;
    return dummy.next;
}

// <array> = <num> "]" ("[" <array>)?
static Type *
parse_array(Token **now, Type *type)
{
    Token *tok = *now;
    int len = get_num_ival(tok);

    tok = tok->next;
    token_assert_consume(&tok, "]");

    if (token_consume(&tok, "["))
        type = parse_array(&tok, type);

    *now = tok;
    return type_array(type, len);
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

    block_scope_enter();

    while (!token_eq(tok, "}"))
    {
        if (is_type_name(tok) || is_user_decl(tok))
            node = node->next = parse_decl(&tok);
        else
            node = node->next = parse_stmt(&tok);
    }

    block_scope_leave();

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

// <mul> = <unary> ("*" <unary> | "/" <unary> | "%" <unary>)*
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
        if (token_eq(tok, "%"))
        {
            tok = tok->next;
            node = new_binary(NT_MOD, node, parse_unary(&tok));
            continue;
        }
        break;
    }

    *now = tok;
    return node;
}

// <unary> = ("+" | "-" | "*" | "&") <unary>
//         | <postfix>
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
        node = parse_postfix(&tok);

    *now = tok;
    return node;
}

// <postfix> = <primary> (<index> | <member>)*
static Node *
parse_postfix(Token **now)
{
    Token *tok = *now;
    Node *node = parse_primary(&tok);

    for (;;)
    {
        if (token_eq(tok, "["))
        {
            node = parse_index(&tok, node);
            continue;
        }
        if (token_eq(tok, "."))
        {
            node = parse_member(&tok, node);
            continue;
        }
        break;
    }

    *now = tok;
    return node;
}

// <index> = "[" <expr> "]"
static Node *
parse_index(Token **now, Node *primary)
{
    Token *tok = *now;
    Node *node = primary;
    Node *expr;

    token_assert_consume(&tok, "[");
    expr = parse_expr(&tok);
    token_assert_consume(&tok, "]");
    node = new_binary(NT_INDEX, node, expr);

    *now = tok;
    return node;
}

// <member> = "." <ident>
static Node *
parse_member(Token **now, Node *primary)
{
    Token *tok = *now;
    Node *node = primary;

    token_assert_consume(&tok, ".");
    node = new_unary(NT_MEMBER, node);
    node->mem_name = get_ident_str(tok);

    *now = tok->next;
    return node;
}

// <primary> = "(" <expr> ")"
//           | <member>
//           | <fn_call>
//           | <var>
//           | <num>
//           | <bval>
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
        if (is_fn_call(tok))
            node = parse_fn_call(&tok);
        else // variable
            node = parse_var(&tok);
        *now = tok;
        return node;
    }
    if (tok->type == TT_NUM)
    {
        *now = tok->next;
        return node_num(tok->ival);
    }
    if (tok->type == TT_KEYWORD && (token_eq(tok, "true") || token_eq(tok, "false")))
    {
        *now = tok->next;
        return node_bval(token_eq(tok, "true") ? 1 : 0);
    }

    die("bad primary from token %d", tok->type);
}

// <fn_call> = (<scope_path>)? <ident> "(" (<assign> ("," <assign>)*)? ")"
static Node *
parse_fn_call(Token **now)
{
    Token *tok = *now;
    Node dummy= {};
    Node *node = &dummy;
    Path *path = NULL;
    const char *fn_name;

    // parse fn scopes
    if (is_scope_path(tok))
        path = parse_scope_path(&tok);

    fn_name = get_ident_str(tok);
    tok = tok->next;

    // parse fn args
    token_assert_consume(&tok, "(");
    while (!token_eq(tok, ")"))
    {
        token_consume(&tok, ",");
        node = node->next = parse_assign(&tok);
    }

    token_assert_consume(&tok, ")");

    node = new_node(NT_FN_CALL);
    node->path = path;
    node->fn_name = fn_name;
    node->fn_args = dummy.next;

    *now = tok;
    return node;
}

// <var> = (<scope_path>)? <ident>
static Node *
parse_var(Token **now)
{
    Token *tok = *now;
    Node *node;
    Path *path = NULL;

    // parse variable scopes
    if (is_scope_path(tok))
        path = parse_scope_path(&tok);

    node = node_var(get_ident_str(tok));
    node->path = path;

    // an unscoped variable can be local var or root global var
    // if we can't find as local var now, find as global var during semantics
    if (!node->path)
        node->var = find_local(node->var_name, strlen(node->var_name));

    *now = tok->next;
    return node;
}

// <scope_path> = (<ident> ":")*
static Path *
parse_scope_path(Token **now)
{
    Token *tok = *now;
    Path dummy= {};
    Path *path = &dummy; 

    while (token_eq(tok->next, ":"))
    {
        path = path->next = new_path(get_ident_str(tok));
        tok = tok->next->next;
    }

    *now = tok;
    return dummy.next;
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

// recursively determine the data type of given Node and check semantics.
// we pass in a Node ** because we may mutate the AST in the process.
static void
sem(Node **node_)
{
    Node *node = *node_;

    if (!node || node->dt)
        return;

    sem(&node->lch);
    sem(&node->rch);
    sem(&node->init);
    sem(&node->cond);
    sem(&node->iter);
    sem(&node->br_if);
    sem(&node->br_else);
    for (Node **stmt = &node->block; *stmt; stmt = &(*stmt)->next)
        sem(stmt);
    for (Node **arg = &node->fn_args; *arg; arg = &(*arg)->next)
        sem(arg);

    // annotate data type for statement nodes and below
    Obj *fn;
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
            return;
        case NT_SUB:
            sem_sub(node_);
            return;
        case NT_MUL:
            sem_mul(node_);
            return;
        case NT_DIV:
            sem_div(node_);
            return;
        case NT_MOD:
            sem_mod(node_);
            return;
        case NT_NEG:
            node->dt = node->lch->dt;
            return;
        case NT_ASSIGN:
            if (is_arr(node->lch->dt))
                die("can not assign to array");
            node->dt = node->lch->dt;
            return;
        case NT_INDEX:
            sem_index(node_);
            return;
        case NT_MEMBER:
            sem_member(node_);
            return;
        case NT_DEREF:
            sem_deref(node_);
            return;
        case NT_ADDR:
            if (is_arr(node->lch->dt))
                node->dt = type_pointer(node->lch->dt->base);
            else
                node->dt = type_pointer(node->lch->dt);
            return;
        case NT_VAR:
            sem_var(node_);
            return;
        case NT_EQ:
        case NT_NE:
        case NT_LT:
        case NT_LE:
        case NT_BVAL:
            node->dt = copy_type(&type_bool);
            return;
        case NT_NUM:
            node->dt = copy_type(&type_i64);
            return;
        case NT_FN_CALL:
            fn = find_fn(node->fn_name, strlen(node->fn_name), node->path);
            if (!fn)
                die("function %s not found", node->fn_name);
            node->dt = fn->dt->ret;
            node->scope = scope_lookup(node->path);
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

    // int + int
    if (is_int(lch->dt) && is_int(rch->dt))
    {
        node->dt = node->lch->dt;
        return;
    }
    // canonicalize int + ptr
    if (is_int(lch->dt) && is_ptr(rch->dt))
    {
        Node *tmp = node->lch;
        node->lch = node->rch;
        node->rch = tmp;
    }
    // ptr + int
    if (is_ptr(lch->dt) && is_int(rch->dt))
    {
        node->rch = new_binary(NT_MUL, rch, node_num(lch->dt->base->size));
        node->rch->dt = copy_type(&type_i64);
        node->dt = node->lch->dt;
        return;
    }

    die("bad add between %d and %d", lch->dt->type, rch->dt->type);
}

static void
sem_sub(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch, *rch = node->rch;

    // int - int
    if (is_int(lch->dt) && is_int(rch->dt))
    {
        node->dt = node->lch->dt;
        return;
    }
    // ptr - int
    if (is_ptr(lch->dt) && is_int(rch->dt))
    {
        node->rch = new_binary(NT_MUL, rch, node_num(lch->dt->base->size));
        node->rch->dt = copy_type(&type_i64);
        node->dt = node->lch->dt;
        return;
    }
    // ptr - ptr
    if (is_ptr(lch->dt) && is_ptr(rch->dt))
    {
        *node_ = new_binary(NT_DIV, node, node_num(lch->dt->base->size));
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

    if (is_int(lch->dt) && is_int(rch->dt))
    {
        node->dt = node->lch->dt;
        return;
    }

    die("bad mul between %d and %d", lch->dt->type, rch->dt->type);
}

static void
sem_div(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch, *rch = node->rch;

    if (is_int(lch->dt) && is_int(rch->dt))
    {
        node->dt = node->lch->dt;
        return;
    }

    die("bad div between %d and %d", lch->dt->type, rch->dt->type);
}

static void
sem_mod(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch, *rch = node->rch;

    if (is_int(lch->dt) && is_int(rch->dt))
    {
        node->dt = node->lch->dt;
        return;
    }

    die("bad mod between %d and %d", lch->dt->type, rch->dt->type);
}

static void
sem_index(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch, *rch = node->rch;

    // given pointer type x, x[y] is alias for *(x+y)
    if (is_ptr(lch->dt))
    {
        node = new_unary(NT_DEREF, new_binary(NT_ADD, lch, rch));
        sem(&node);
        *node_ = node;
        return;
    }

    die("bad index between %d and %d", lch->dt->type, rch->dt->type);
}

static void
sem_member(Node **node_)
{
    Node *node = *node_;
    Node *lch = node->lch;

    // find struct type by path and name
    if (lch->dt->type != DT_STRUCT)
        die("failed to access member of non-struct type %d", lch->dt->type);

    // find member of struct
    node->mem = NULL;
    for (Obj *mem = lch->dt->members; mem; mem = mem->next)
        if (!strcmp(node->mem_name, mem->name))
        {
            node->mem = mem;
            break;
        }
    if (!node->mem)
        die("failed to access member %s of struct %s", node->mem_name, lch->dt->name);

    node->dt = node->mem->dt;

    if (is_arr(node->dt))
    {
        node = new_unary(NT_ADDR, node);
        sem(&node);
        *node_ = node;
    }
}

static void
sem_deref(Node **node_)
{
    Node *node = *node_;

    if (!node->lch->dt->base)
        die("attempt to deref a non-pointer");

    node->dt = node->lch->dt->base;

    if (is_arr(node->dt))
    {
        node = new_unary(NT_ADDR, node);
        sem(&node);
        *node_ = node;
    }
}

static void
sem_var(Node **node_)
{
    Node *node = *node_;
    Obj *var;

    if (node->var) // local variable
        node->dt = node->var->dt;
    else // global variable
    {
        var = find_global(node->var_name, strlen(node->var_name), node->path);
        if (!var)
            die("identifier %s not declared", node->var_name);
        node->scope = scope_lookup(node->path);
        node->var = var;
        node->dt = var->dt;
    }

    // deduce user defined type now
    deduce_user_type(&node->dt);

    // array variable is converted into a pointer to it's first element
    if (is_arr(node->dt))
    {
        node = new_unary(NT_ADDR, node);
        sem(&node);
        *node_ = node;
    }
}
