#include "test.h"

bool
test_flow()
{
    puts(__FILE__);

    // if branch
    TEST(1,  "fn main() i64 { if 1 {return 1;} return 2; }");
    TEST(2,  "fn main() i64 { if 0 {return 1;} return 2; }");

    // if else branch
    TEST(1,  "fn main() i64 { if 1 {return 1;} else {return 2;} return 3;}");
    TEST(2,  "fn main() i64 { if 0 {return 1;} else {return 2;} return 3;}");

    // if + else if
    TEST(1,  "fn main() i64 { if 1 {return 1;} else if 1 {return 2;} return 3;}");
    TEST(2,  "fn main() i64 { if 0 {return 1;} else if 1 {return 2;} return 3;}");
    TEST(3,  "fn main() i64 { if 0 {return 1;} else if 0 {return 2;} return 3;}");

    // if + else if + else
    TEST(1,  "fn main() i64 { if 1 {return 1;} else if 1 {return 2;} else {return 3;} return 4;}");
    TEST(2,  "fn main() i64 { if 0 {return 1;} else if 1 {return 2;} else {return 3;} return 4;}");
    TEST(3,  "fn main() i64 { if 0 {return 1;} else if 0 {return 2;} else {return 3;} return 4;}");
    TEST(4,  "fn main() i64 { if 0 {return 1;} else if 0 {return 2;} else {} return 4;}");

    // standard 3-part for loop
    TEST(55, "fn main() i64 { i64 s=0, i;   for i=1; i<=10; i=i+1 { s=s+i; } return s; }");
    TEST(55, "fn main() i64 { i64 s=0, i=1; for ;    i<=10; i=i+1 { s=s+i; } return s; }");
    TEST(55, "fn main() i64 { i64 s=0, i=1; for ;; i=i+1 { s=s+i; if i==10 { return s; } } }");

    // while loop
    TEST(55, "fn main() i64 { i64 s=0, i=1; for   i<=10   { s=s+i; i=i+1; } return s; }");
    TEST(55, "fn main() i64 { i64 s=0, i=1; for ; i<=10 ; { s=s+i; i=i+1; } return s; }");

    // infinite loop
    TEST(55, "fn main() i64 { i64 s=0, i=1; for    { s=s+i; i=i+1; if i>10 {return s;} } }");
    TEST(55, "fn main() i64 { i64 s=0, i=1; for ;; { s=s+i; i=i+1; if i>10 {return s;} } }");

    puts("ok");
    return true;
}
