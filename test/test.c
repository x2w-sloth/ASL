#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "test.h"

typedef bool (*test_fn)(void);

bool
str_test(int val, const char *src)
{
    static char cmd[512];
    int status;

    printf("%s", src);

    // compile ASL source string
    snprintf(cmd, sizeof(cmd), "./aslc \"%s\" > tmp.s", src);
    assert(system(cmd) == 0);
    assert(system("as tmp.s -o tmp.o -msyntax=intel -mnaked-reg") == 0);
    assert(system("ld tmp.o -o tmp") == 0);

    // run compiled binary
    status = system("./tmp") >> 8;
    printf(" => %d", status);

    if (val != status)
    {
        printf(", expected %d\n", val);
        return false;
    }
    putchar('\n');
    return true;
}

int
main()
{
    test_fn tests[] = {
        test_cmp,
        test_expr,
        test_flow,
        test_locals,
        test_globals,
        test_pointer,
        test_function,
    };

    for (int i = 0; i < COUNT(tests); i++)
        if (!tests[i]())
        {
            puts("fail");
            exit(EXIT_FAILURE);
        }

    puts("\nall ok");
    return EXIT_SUCCESS;
}
