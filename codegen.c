#include <string.h>
#include "aslc.h"
#include "util.h"

static void println(const char *fmt, ...);
static void gen_data(Scope *sc);
static void gen_text(Scope *sc);
static void gen_fn(Obj *fn);
static void gen_stmt(Node *node);
static void gen_expr(Node *node);
static void gen_addr(Node *node);
static void push();
static void pop(const char *reg);
static void load(const char *reg, Type *dt);
static void store(const char *reg);
static char *get_fullname(const char *name, const Scope *sc);
static void assign_locals(Obj *fn);
static int align_to(int n, int align);
static int new_id();

static FILE *genfile;
static int depth;
static Obj *fn_now;
static const char *arg64[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

void
gen(Scope *sc_root)
{
    genfile = stdout;

    // .data section
    println("  .data");
    gen_data(sc_root);

    // .text section
    println("  .text");
    gen_text(sc_root);

    // entry point
    println("  .globl _start");
    println("_start:");
    println("  call main");
    println("  mov  rdi, rax");
    println("  mov  rax, 0x3C");
    println("  syscall");
}

static void
println(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(genfile, fmt, ap);
    va_end(ap);

    fputc('\n', genfile);
}

static void
gen_data(Scope *sc)
{
    if (!sc)
        return;

    for (Scope *s = sc; s; s = s->next)
        for (Obj *obj = s->globals; obj; obj = obj->next)
        {
            const char *name = get_fullname(obj->name, s);
            println("  .globl %s", name);
            println("%s:", name);
            println("  .zero %d", obj->dt->size);
        }
    gen_data(sc->children);
}

static void
gen_text(Scope *sc)
{
    if (!sc)
        return;

    for (Scope *s = sc; s; s = s->next)
        for (Obj *obj = s->fns; obj; obj = obj->next)
            gen_fn(obj);
    gen_text(sc->children);
}

static void
gen_fn(Obj *fn)
{
    const char *name = get_fullname(fn->name, fn->scope);

    fn_now = fn;
    assign_locals(fn);

    // fn prologue
    println("  .globl %s", name);
    println("%s:", name);
    println("  push rbp");
    println("  mov  rbp, rsp");
    println("  sub  rsp, %d", fn->stack_size);

    // move parameters to stack
    int i = 0;
    for (Obj *param = fn->params; param; param = param->next)
        println("  mov  [rbp - %d], %s", param->rbp_off, arg64[i++]);

    for (Node *stmt = fn->body; stmt; stmt = stmt->next)
    {
        gen_stmt(stmt);
        if (depth != 0)
            die("bad stack depth %d", depth);
    }

    // fn epilogue
    println("ret.%s:", name);
    println("  mov  rsp, rbp");
    println("  pop  rbp");
    println("  ret");
}

static void
gen_stmt(Node *node)
{
    int id;

    switch (node->type)
    {
        case NT_RET_STMT:
            gen_stmt(node->lch);
            println("  jmp  ret.%s", get_fullname(fn_now->name, fn_now->scope));
            break;
        case NT_BLOCK_STMT:
            for (Node *stmt = node->block; stmt; stmt = stmt->next)
                gen_stmt(stmt);
            break;
        case NT_EXPR_STMT:
            gen_expr(node->lch);
            break;
        case NT_IF_STMT:
            id = new_id();
            gen_expr(node->cond);
            println("  test rax, rax");
            if (node->br_else)
            {
                println("  jz   else.%d", id);
                gen_stmt(node->br_if);
                println("  jmp  end.%d", id);
                println("else.%d:", id);
                gen_stmt(node->br_else);
            }
            else
            {
                println("  jz   end.%d", id);
                gen_stmt(node->br_if);
            }
            println("end.%d:", id);
            break;
        case NT_FOR_STMT:
            id = new_id();
            if (node->init)
                gen_stmt(node->init);
            println("for.body.%d:", id);
            if (node->cond)
            {
                gen_expr(node->cond);
                println("  test rax, rax");
                println("  jz   for.exit.%d", id);
            }
            gen_stmt(node->block);
            if (node->iter)
                gen_expr(node->iter);
            println("  jmp  for.body.%d", id);
            println("for.exit.%d:", id);
            break;
        default:
            die("bad stmt node %d", node->type);
    }
}

static void
gen_expr(Node *node)
{
    int nargs = 0;

    switch (node->type)
    {
        case NT_NUM:
            println("  mov  rax, %d", node->ival);
            return;
        case NT_VAR:
            gen_addr(node);
            load("rax", node->dt);
            return;
        case NT_ASSIGN:
            gen_addr(node->lch);
            push();
            gen_expr(node->rch);
            pop("rdi");
            store("rdi");
            return;
        case NT_FN_CALL:
            for (Node *arg = node->fn_args; arg; arg = arg->next)
            {
                gen_expr(arg);
                push();
                if (++nargs > 6)
                    die("no support for fn with more than 6 args");
            }
            for (int i = nargs - 1; i >= 0; i--)
                pop(arg64[i]);
            println("  xor  rax, rax");
            println("  call %s", get_fullname(node->fn_name, node->scope));
            return;
        case NT_DEREF:
            gen_expr(node->lch);
            load("rax", node->dt);
            return;
        case NT_ADDR:
            gen_addr(node->lch);
            return;
        case NT_NEG:
            gen_expr(node->lch);
            println("  neg  rax");
            return;
        default:
            break;
    }

    gen_expr(node->rch);
    push();
    gen_expr(node->lch);
    pop("rdi");

    switch (node->type)
    {
        case NT_ADD:
            println("  add  rax, rdi");
            break;
        case NT_SUB:
            println("  sub  rax, rdi");
            break;
        case NT_MUL:
            println("  imul rdi");
            break;
        case NT_DIV:
            println("  idiv rdi");
            break;
        case NT_EQ:
        case NT_NE:
        case NT_LT:
        case NT_LE:
            println("  cmp  rax, rdi");
            if (node->type == NT_EQ)
                println("  sete al");
            if (node->type == NT_NE)
                println("  setne al");
            if (node->type == NT_LT)
                println("  setl al");
            if (node->type == NT_LE)
                println("  setle al");
            break;
        default:
            die("bad expr node %d", node->type);
    }
}

static void
gen_addr(Node *node)
{
    switch (node->type)
    {
        case NT_VAR:
            if (node->var->type == OT_LOCAL)
                println("  lea  rax, [rbp - %d]", node->var->rbp_off);
            else if (node->var->type == OT_GLOBAL)
                println("  lea  rax, [%s]", get_fullname(node->var_name, node->scope));
            else
                die("bad variable type %d", node->var->type);
            break;
        case NT_DEREF:
            gen_expr(node->lch);
            break;
        default:
            die("bad addr node %d", node->type);
    }
}

static void
push()
{
    println("  push rax");
    ++depth;
}

static void
pop(const char *reg)
{
    println("  pop  %s", reg);
    --depth;
}

// load value to rax from pointer location
static void
load(const char *reg, Type *dt)
{
    if (dt->type == DT_ARR)
    {
        if (strcmp(reg, "rax"))
            println("  mov rax, %s", reg);
        return; // pointer value is now loaded at rax
    }
    println("  mov  rax, [%s]", reg);
}

// write value from rax to pointer location
static void
store(const char *reg)
{
    println("  mov  [%s], rax", reg);
}

static char *
get_fullname(const char *name, const Scope *sc)
{
    static char buf[512];

    strncpy(buf, name, 512);
    for (int i = strlen(name); i < 512 && sc && sc->name; i += strlen(sc->name), sc = sc->parent)
    {
        buf[i++] = '.';
        strncpy(buf + i, sc->name, 512 - i);
    }

    return buf;
}

static void
assign_locals(Obj *fn)
{
    int offset = 0;

    for (Obj *local = fn->locals; local; local = local->next)
    {
        offset += local->dt->size;
        local->rbp_off = offset;
    }

    fn->stack_size = align_to(offset, 16);
}

static int
align_to(int n, int align)
{
    return (n + align - 1) / align * align;
}

static int
new_id()
{
    static int id = 0;
    return id++;
}
