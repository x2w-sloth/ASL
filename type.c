#include "aslc.h"

Type type_i64 = { .type = DT_INT, .bits = 64 };

void
add_dt(Node *node)
{
    if (!node || node->dt)
        return;

    add_dt(node->lch);
    add_dt(node->rch);

    for (Node *stmt = node->block; stmt; stmt = stmt->next)
        add_dt(stmt);

    // annotate data type for statement nodes and below
    switch (node->type)
    {
        case NT_RET_STMT:
        case NT_BLOCK_STMT:
        case NT_EXPR_STMT:
            return;
        case NT_ADD:
        case NT_SUB:
        case NT_MUL:
        case NT_DIV:
        case NT_NEG:
        case NT_ASSIGN:
            node->dt = node->lch->dt;
            return;
        case NT_EQ:
        case NT_NE:
        case NT_LT:
        case NT_LE:
        case NT_VAR:
        case NT_NUM:
        case NT_FN_CALL:
            node->dt = &type_i64;
            return;
        default:
            die("can not annotate data type for node %d", node->type);
    }
}

bool
is_int(const Type *dt, int bits)
{
    return dt->type == DT_INT && dt->bits == bits;
}
