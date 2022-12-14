#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../util.h"
#include "test.h"

typedef bool (*test_fn)(void);

static bool validate(int val);

bool
str_test(int val, char *src)
{
    static char cmd[512];

    printf("  %s", src);

    // compile ASL source string
    snprintf(cmd, sizeof(cmd), "echo \"%s\" | ./aslc - -o tmp", src);

    if (system(cmd) != 0)
      die("failed to compile program");

    return validate(val);
}

bool
file_test(int val, char *path)
{
    static char cmd[512];

    printf("  %s", path);

    // compile ASL source file
    snprintf(cmd, sizeof(cmd), "./aslc %s -o tmp", path);

    if (system(cmd) != 0)
      die("failed to compile program");

    return validate(val);
}

static bool
validate(int val)
{
    int status;
    char *bin[] = { "./tmp", NULL };

    status = runcmd(bin) >> 8;
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
        test_arrays,
        test_structs,
        test_globals,
        test_scopes,
        test_pointer,
        test_function,
        test_integration,
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
