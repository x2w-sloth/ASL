#include "aslc.h"

static void gen_expr(Node *root);
static void push();
static void pop(const char *reg);
static int depth;

void
gen(Node *root)
{
    println("  .globl _start");
    println("_start:");

    gen_expr(root);

    if (depth != 0)
        die("bad stack depth %d", depth);

    println("  mov  rdi, rax");
    println("  mov  rax, 0x3C");
    println("  syscall");
}

static void
gen_expr(Node *node)
{
    switch (node->type)
    {
        case NT_NUM:
            println("  mov  rax, %d", node->ival);
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