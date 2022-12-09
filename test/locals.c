#include "test.h"

bool
test_locals()
{
    puts(__FILE__);

    // bool locals
    TEST(1, "fn main() i64 {return true;}");
    TEST(0, "fn main() i64 {return false;}");
    TEST(1, "fn main() i64 {bool b=true; return b;}");
    TEST(0, "fn main() i64 {bool b=false; return b;}");

    // i64 locals
    TEST(3, "fn main() i64 {i64 a=1+2; return a;}");
    TEST(2, "fn main() i64 {i64 a=3, z=5; return z-a;}");
    TEST(2, "fn main() i64 {i64 a1; i64 a2; a1=a2=1; return a1+a2;}");
    TEST(2, "fn main() i64 {i64 a=5,b=5; i64 c=5; return (a+b)/c;}");
    TEST(3, "fn main() i64 {i64 var; return var=3;}");

    // i8 locals
    TEST(3, "fn main() i64 {i8 a=1+2; return a;}");
    TEST(2, "fn main() i64 {i8 a=3, z=5; return z-a;}");
    TEST(2, "fn main() i64 {i8 a1; i8 a2; a1=a2=1; return a1+a2;}");
    TEST(2, "fn main() i64 {i8 a=5,b=5; i8 c=5; return (a+b)/c;}");
    TEST(3, "fn main() i64 {i8 var; return var=3;}");

    // integer mix
    TEST(7, "fn main() i64 { i8 a=1; i64 b=2; i8 c=4; return a+b+c;}");
    TEST(7, "fn main() i64 { i8 a=1, b=2; i64 c=4; return a+b+c;}");
    TEST(7, "fn main() i64 { i64 a=1, b=2; i8 c=4; return a+b+c;}");
    TEST(7, "fn main() i64 { i64 a=1; i32 b=2; i8 c=4; return a+b+c;}");
    TEST(7, "fn main() i64 { i64 a=1; i32 b=2, c=4; return a+b+c;}");
    TEST(7, "fn main() i64 { i32 a=1, b=2; i8 c=4; return a+b+c;}");

    puts("ok");
    return true;
}
