#include "test.h"

bool
test_cmp()
{
    puts(__FILE__);

    TEST(0, "fn main() i64 {return 0==1;}");
    TEST(1, "fn main() i64 {return 42==42;}");
    TEST(1, "fn main() i64 {return 0!=1;}");
    TEST(0, "fn main() i64 {return 2!=2;}");
    TEST(1, "fn main() i64 {return 0<1;}");
    TEST(0, "fn main() i64 {return 1<1;}");
    TEST(0, "fn main() i64 {return 2<1;}");
    TEST(1, "fn main() i64 {return 0<=1;}");
    TEST(1, "fn main() i64 {return 1<=1;}");
    TEST(0, "fn main() i64 {return 2<=1;}");
    TEST(1, "fn main() i64 {return 1>0;}");
    TEST(0, "fn main() i64 {return 1>1;}");
    TEST(0, "fn main() i64 {return 1>2;}");
    TEST(1, "fn main() i64 {return 1>=0;}");
    TEST(1, "fn main() i64 {return 1>=1;}");
    TEST(0, "fn main() i64 {return 1>=2;}");
    TEST(1, "fn main() i64 {return (1+2)*3 == 18/2;}");

    puts("ok");
    return true;
}
