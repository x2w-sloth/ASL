#include "test.h"

bool
test_function()
{
    puts(__FILE__);

    TEST(4,  "fn main() i64 {return get4();} fn get4() i64 {return 2 * 2;}");
    TEST(10, "fn main() i64 {return get2() * get5();} fn get2() i64 {return 2;} fn get5() i64 {return 5;}");
    TEST(1,  "fn main() i64 {return sub(2, 1);} fn sub(i64 a, i64 b) i64 {return a - b;}");
    TEST(2,  "fn main() i64 {return sub(3,sub(2, 1));} fn sub(i64 a, i64 b) i64 {return a - b;}");
    TEST(5,  "fn main() i64 {return add3(2);} fn add3(i64 a) i64 {i64 b=3; return a+b;}");
    TEST(21, "fn main() i64 {return sum(1,2,3,4,5,6);} fn sum(i64 a,i64 b,i64 c,i64 d,i64 e,i64 f) i64 {return a+b+c+d+e+f;}");
    TEST(4,  "fn main() i64 {i64 x=3; foo(&x); return x;} fn foo(i64* x) {*x=4;}");

    puts("ok");
    return true;
}
