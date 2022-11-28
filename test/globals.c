#include "test.h"

bool
test_globals()
{
    puts(__FILE__);

    TEST(3, "i64 x; fn main() i64 { return x=3; }");
    TEST(5, "i64 x,y; fn main() i64 { y=2; return x=3 + y; }");
    TEST(9, "i64 x,y; i64 z; fn main() i64 { x=2;y=3;z=4; return x+y+z; }");
    TEST(9, "i64 x; fn main() i64 { i64 y=7; x=2; return x+y; }");

    puts("ok");
    return true;
}

