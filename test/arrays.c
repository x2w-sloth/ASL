#include "test.h"

bool
test_arrays()
{
    puts(__FILE__);

    // index operator on pointers
    TEST(1, "fn main() i64 {i64 a=1, b=2; i64* p=&a; return p[0];}");
    TEST(2, "fn main() i64 {i64 a=1, b=2; i64* p=&a; return p[1];}");
    TEST(2, "fn main() i64 {i64 a=1, b=2; i64* p=&a; return p[b-1];}");
    TEST(4, "fn main() i64 {i64 a=1, b=2, c=3; i64* p=&b; return p[-1] + p[1];}");
    TEST(1, "fn main() i64 {i64 a=1; i64* p=&a; i64** pp=&p; return pp[0][0];}");

    // one dimensional arrays
    TEST(2, "fn main() i64 {i64 a[1]; *a=2; return a[0];}");
    TEST(3, "fn main() i64 {i64 a[1]; i64* p=a; *p=3; return a[0];}");
    TEST(3, "fn main() i64 {i64 a[2]; a[0]=1; a[1]=2; return a[0] + a[1];}");
    TEST(3, "fn main() i64 {i64 a[64]; *(a+63)=2; a[63]=a[63]+1; return a[63];}");

    // multi dimensional arrays
    TEST(0, "fn main() i64 {i64 a[3][2]; i64* p=a; *(p+0)=0; return a[0][0];}");
    TEST(1, "fn main() i64 {i64 a[3][2]; i64* p=a; *(p+1)=1; return a[0][1];}");
    TEST(2, "fn main() i64 {i64 a[3][2]; i64* p=a; *(p+2)=2; return a[1][0];}");
    TEST(3, "fn main() i64 {i64 a[3][2]; i64* p=a; *(p+3)=3; return a[1][1];}");
    TEST(4, "fn main() i64 {i64 a[3][2]; i64* p=a; *(p+4)=4; return a[2][0];}");
    TEST(5, "fn main() i64 {i64 a[3][2]; i64* p=a; *(p+5)=5; return a[2][1];}");
    TEST(0, "fn main() i64 {i64 a[3][2]; a[0][0]=0; return a[0][0];}");
    TEST(1, "fn main() i64 {i64 a[3][2]; a[0][1]=1; return a[0][1];}");
    TEST(2, "fn main() i64 {i64 a[3][2]; a[1][0]=2; return a[1][0];}");
    TEST(3, "fn main() i64 {i64 a[3][2]; a[1][1]=3; return a[1][1];}");
    TEST(4, "fn main() i64 {i64 a[3][2]; a[2][0]=4; return a[2][0];}");
    TEST(5, "fn main() i64 {i64 a[3][2]; a[2][1]=5; return a[2][1];}");

    puts("ok");
    return true;
}
