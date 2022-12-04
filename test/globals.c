#include "test.h"

bool
test_globals()
{
    puts(__FILE__);

    TEST(3, "i64 x; fn main() i64 { return x=3; }");
    TEST(4, "i64 x; fn main() i64 { i64 x=4; return x; }");
    TEST(5, "i64 x,y; fn main() i64 { y=2; return x=3 + y; }");
    TEST(9, "i64 x,y; i64 z; fn main() i64 { x=2;y=3;z=4; return x+y+z; }");
    TEST(9, "i64 x; fn main() i64 { i64 y=7; x=2; return x+y; }");

    TEST(5, "fn main() i64 { y=2; return x=3 + y; } i64 x, y;");
    TEST(9, "fn main() i64 { x=2;y=3;z=4; return x+y+z; } i64 x,y; i64 z;");
    TEST(9, "fn main() i64 { i64 y=7; x=2; return x+y; } i64 x;");
    TEST(3, "fn main() i64 { a[0]=1; a[1]=2; return *a + a[1]; } i64 a[2];");
    
    puts("ok");
    return true;
}

