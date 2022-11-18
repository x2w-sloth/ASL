#include "aslc.h"

static void gen_expr(Node *root);

void
gen(Node *root)
{
    println("  .globl _start");
    println("_start:");

    gen_expr(root);

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
            break;
        default:
            die("bad expr node %d", node->type);
    }
}
