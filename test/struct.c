#include "test.h"

bool
test_structs()
{
    puts(__FILE__);

    TEST(3,  "fn main() i64 { Foo f; f.x=3; return f.x; }"
             "   struct Foo { i64 x; }");
    TEST(4,  "fn main() i64 { Foo f[2]; f[0].x=1; f[1].x=3; return f[0].x + f[1].x; }"
             "   struct Foo { i64 x; }");
    TEST(5,  "fn main() i64 { Foo f; f.x=2; f.y=3; return f.x + f.y; }"
             "   struct Foo { i64 x,y; }");
    TEST(7,  "fn main() i64 { Foo f; f.a[0]=1; f.a[1]=2; f.a[2]=4; return f.a[0]+f.a[1]+f.a[2]; }"
             "   struct Foo { i64 a[3]; }");
    TEST(10, "fn main() i64 { Foo f; i64* p = f.a; *p = 10; return f.a[0]; }"
             "   struct Foo { i64 a[1]; }");
    TEST(10, "fn main() i64 { Foo f; f.p = &f.a; *(f.p) = 10; return f.a; }"
             "   struct Foo { i64 a; i64* p; }");

    // nested structs
    TEST(2,  "fn main() i64 { Foo f; f.b1.x = 2; f.b2.x = f.b1.x; return f.b2.x; }"
             "   struct Foo { Bar b1, b2; } struct Bar { i64 x; }");
    TEST(5,  "fn main() i64 { Foo f; f.b[0].x = 2; f.b[1].x = 3; return f.b[0].x + f.b[1].x; }"
             "   struct Foo { Bar b[2]; } struct Bar { i64 x; }");
    TEST(3,  "fn main() i64 { Foo f; f.b.j.x = 3; return f.b.j.x; }"
             "   struct Foo { Bar b; } struct Bar { Jar j; } struct Jar { i64 x; }");

    // scoped structs
    TEST(9,  "fn main() i64 { s:s1 a; a.x=9; s:s2 b; b.y=a.x; return b.y; }"
             "scope s { struct s1 { i8 x; } struct s2 { i8 y; } }");
    TEST(5,  "fn main() i64 { s:A a; a.x = 1; B b; b.a.x = 4; return a.x + b.a.x; }"
             "scope s { struct A { i8 x; } } struct B { s:A a; }");
    TEST(2,  "fn main() i64 { m:vec2 v; v.x=2; v.y=3; v.bar=true;"
             "if v.bar { return v.x; } else { return v.y; } }"
             "scope m { struct vec2 { i64 x,y; bool bar; } }");

    puts("ok");
    return true;
}

