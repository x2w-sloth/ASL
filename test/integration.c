#include "test.h"

bool
test_integration()
{
    puts(__FILE__);

    TEST_FILE(6,  "./test/asl/minmax.asl");
    TEST_FILE(85, "./test/asl/sumprod.asl");

    puts("ok");
    return true;
}
