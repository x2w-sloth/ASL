#include "test.h"

bool
test_expr()
{
    puts(__FILE__);

    TEST(0,  "fn main() i64 {return 0;}");
    TEST(4,  "fn main() i64 {return 4;}");
    TEST(8,  "fn main() i64 {4; return 8;}");
    TEST(12, "fn main() i64 {4;6; return 12;}");
    TEST(30, "fn main() i64 {return 30; return 31;}");
    TEST(3,  "fn main() i64 {return 1+2;}");
    TEST(6,  "fn main() i64 {return 1+2 + 3;}");
    TEST(10, "fn main() i64 {return 30-20;}");
    TEST(33, "fn main() i64 {return 30 - 3 + 6;}");
    TEST(20, "fn main() i64 {return 4+4*4;}");
    TEST(8,  "fn main() i64 {return 16*2/4;}");
    TEST(9,  "fn main() i64 {return 3*6/2;}");
    TEST(4,  "fn main() i64 {return (1+1)*2;}");
    TEST(10, "fn main() i64 {return (2+3) * ((11-1)/5);}");
    TEST(7,  "fn main() i64 {return (22 % 3) * 5 + 2;}");
    TEST(4,  "fn main() i64 {return 50 * 2 % 8;}");
    TEST(0,  "fn main() i64 {return (8+16+24+8888888) % 8;}");
    TEST(4,  "fn main() i64 {return +5 + -1;}");
    TEST(30, "fn main() i64 {return - - -30 + - -60;}");
    TEST(6,  "fn main() i64 {return (-8 + 5)*(- -2 - 4);}");
    TEST(2,  "fn main() i64 {;; return 2;}");
    TEST(1,  "fn main() i64 {{{}} return 1;}");
    TEST(3,  "fn main() i64 {1; {return 3;} return 2;}");

    puts("ok");
    return true;
}
