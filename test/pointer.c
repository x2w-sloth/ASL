#include "test.h"

bool
test_pointer()
{
    puts(__FILE__);

    TEST(10, "fn main() i64 { i64 x=10; return *&x; }");
    TEST(10, "fn main() i64 { i64 x=10; i64* y=&x; i64** z=&y; return **z; }");
    TEST(10, "fn main() i64 { i64 x=15; i64* y=&x; *y=10; return x; }");
    TEST(5,  "fn main() i64 { i64 x=3; i64 y=5; return *(&x+1); }");
    TEST(3,  "fn main() i64 { i64 x=3; i64 y=5; return *(&y-1); }");
    TEST(1,  "fn main() i64 { i64 x=3; i64 y=1; return *(&x-(-y)); }");
    TEST(7,  "fn main() i64 { i64 x, y=5; *(&x+1)=7; return y; }");
    TEST(7,  "fn main() i64 { i64 x=5, y; *(&y-1)=7; return x; }");
    TEST(5,  "fn main() i64 { i64 x; return (&x+3)-(&x-2); }");
    TEST(10, "fn main() i64 { i64 x; return &x + 1 - &x + 9; }");

    puts("ok");
    return true;
}
