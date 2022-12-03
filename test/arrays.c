#include "test.h"

bool
test_arrays()
{
    puts(__FILE__);

    TEST(1, "fn main() i64 {i64 a=1, b=2; i64* p=&a; return p[0];}");
    TEST(2, "fn main() i64 {i64 a=1, b=2; i64* p=&a; return p[1];}");
    TEST(2, "fn main() i64 {i64 a=1, b=2; i64* p=&a; return p[b-1];}");
    TEST(4, "fn main() i64 {i64 a=1, b=2, c=3; i64* p=&b; return p[-1] + p[1];}");
    TEST(1, "fn main() i64 {i64 a=1; i64* p=&a; i64** pp=&p; return pp[0][0];}");

    puts("ok");
    return true;
}
