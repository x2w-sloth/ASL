#include "test.h"

bool
test_locals()
{
    puts(__FILE__);

    TEST(3, "fn main() i64 {i64 a=1+2; return a;}");
    TEST(2, "fn main() i64 {i64 a=3, z=5; return z-a;}");
    TEST(2, "fn main() i64 {i64 a1; i64 a2; a1=a2=1; return a1+a2;}");
    TEST(2, "fn main() i64 {i64 a=5,b=5; i64 c=5; return (a+b)/c;}");
    TEST(3, "fn main() i64 {i64 var; return var=3;}");

    puts("ok");
    return true;
}
