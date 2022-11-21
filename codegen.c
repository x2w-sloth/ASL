#include "aslc.h"

static void gen_fn(Obj *fn);
static void gen_stmt(Node *node);
static void gen_expr(Node *node);
static void gen_addr(Node *node);
static void push();
static void pop(const char *reg);
static void assign_locals(Obj *fn);
static int align_to(int n, int align);

static int depth;
static Obj *fn_now;

void
gen(Obj *prog)
{
    for (Obj *fn = prog; fn; fn = fn->next)
        gen_fn(fn);

    println("  .globl _start");
    println("_start:");
    println("  call main");
    println("  mov  rdi, rax");
    println("  mov  rax, 0x3C");
    println("  syscall");
}

static void
gen_fn(Obj *fn)
{
    fn_now = fn;
    assign_locals(fn);

    // fn prologue
    println("  .globl %s", fn->name);
    println("%s:", fn->name);
    println("  push rbp");
    println("  mov  rbp, rsp");
    println("  sub  rsp, %d", fn->stack_size);

    for (Node *stmt = fn->body; stmt; stmt = stmt->next)
    {
        gen_stmt(stmt);
        if (depth != 0)
            die("bad stack depth %d", depth);
    }

    // fn epilogue
    println("ret.%s:", fn->name);
    println("  mov  rsp, rbp");
    println("  pop  rbp");
    println("  ret");
}

static void
gen_stmt(Node *node)
{
    switch (node->type)
    {
        case NT_RET_STMT:
            gen_stmt(node->lch);
            println("  jmp  ret.%s", fn_now->name);
            break;
        case NT_BLOCK_STMT:
            for (Node *stmt = node->block; stmt; stmt = stmt->next)
                gen_stmt(stmt);
            break;
        case NT_EXPR_STMT:
            gen_expr(node->lch);
            break;
        default:
            die("bad stmt node %d", node->type);
    }
}

static void
gen_expr(Node *node)
{
    switch (node->type)
    {
        case NT_NUM:
            println("  mov  rax, %d", node->ival);
            return;
        case NT_VAR:
            gen_addr(node);
            println("  mov  rax, [rax]");
            return;
        case NT_ASSIGN:
            gen_addr(node->lch);
            push();
            gen_expr(node->rch);
            pop("rdi");
            println("  mov  [rdi], rax");
            return;
        case NT_FN_CALL:
            println("  xor  rax, rax");
            println("  call %s", node->fn_name);
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
            println("  lea  rax, [rbp - %d]", node->var->rbp_off);
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

static void
assign_locals(Obj *fn)
{
    int offset = 8;

    for (Obj *local = fn->locals; local; local = local->next)
    {
        local->rbp_off = offset;
        offset += 8;
    }

    fn->stack_size = align_to(offset, 16);
}

static int
align_to(int n, int align)
{
    return (n + align - 1) / align * align;
}