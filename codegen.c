#include "aslc.h"

static void gen_stmt(Node *node);
static void gen_expr(Node *node);
static void push();
static void pop(const char *reg);
static int depth;

void
gen(Node *root)
{
    println("  .globl _start");
    println("_start:");

    for (Node *stmt = root; stmt; stmt = stmt->next)
    {
        gen_stmt(stmt);
        if (depth != 0)
            die("bad stack depth %d", depth);
    }

    println("  mov  rdi, rax");
    println("  mov  rax, 0x3C");
    println("  syscall");
}

static void
gen_stmt(Node *node)
{
    switch (node->type)
    {
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
        default:
            die("bad expr node %d", node->type);
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
