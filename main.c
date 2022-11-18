#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

_Static_assert (sizeof(int) == 4, "platform int is not 32-bit");

#define println(...)    fprintln(stdout, __VA_ARGS__)
#define eprintln(...)   fprintln(stderr, __VA_ARGS__)
#define die(...)               \
    do {                       \
        eprintln(__VA_ARGS__); \
        exit(EXIT_FAILURE);    \
    } while (0)

static void
fprintln(FILE *file, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(file, fmt, ap);
    va_end(ap);
    fputc('\n', file);
}

// generate assembly for GNU assembler and linker
static void
gen(int num)
{
    println("  .globl _start");
    println("_start:");
    println("  mov  rax, %d", num);
    println("  mov  rdi, rax");
    println("  mov  rax, 0x3C");
    println("  syscall");
}

int
main(int argc, char **argv)
{
    if (argc != 2)
        die("usage: %s <source>", argv[0]);

    int num = atoi(argv[1]);
    gen(num);

    return 0;
}
