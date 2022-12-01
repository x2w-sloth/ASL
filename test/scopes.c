#include "test.h"

bool
test_scopes()
{
    puts(__FILE__);

    // block scopes
    TEST(2, "fn main() i64 { i64 x; { x=2; } return x; }");
    TEST(5, "fn main() i64 { i64 x; { i64 y=3; x=2+y; } return x; }");
    TEST(6, "fn main() i64 { i64 x; { i64 y=3; x=2+y; } { i64 y=1; x=x+y; } return x; }");
    TEST(3, "fn main() i64 { i64 x=3; {{{}}} return x; }");
    TEST(1, "fn main() i64 { i64 x=3; {{{ x=1; }}} return x; }");
    TEST(6, "fn main() i64 { i64 x=3; {i64 y=2; {i64 z=1; { x=x+y+z; }}} return x; }");

    // named scopes
    TEST(3, "scope global {i64 x;} fn main() i64 { return global:x=3; }");
    TEST(6, "scope g {i64 x; i64 y;} fn main() i64 { g:x=3; g:y=2; return g:x * g:y; }");
    TEST(5, "scope a {i64 x;} scope b {i64 x;} fn main() i64 { a:x=2; b:x=3; return a:x + b:x; }");
    TEST(3, "scope a { scope b { i64 c; } } fn main() i64 { a:b:c = 3; return a:b:c; }");
    TEST(7, "fn main() i64 { i64 x=4; g:x=3; return x + g:x; } scope g {i64 x;}");
    TEST(3, "fn main() i64 { return g:x=3; } scope g {i64 x;}");
    TEST(5, "fn main() i64 { a:x=2; b:x=3; return a:x + b:x; } scope a {i64 x;} scope b {i64 x;}");
    TEST(9, "fn main() i64 { a:a:x=4; a:b:x=5; return a:a:x + a:b:x; } scope a { scope a {i64 x;} scope b {i64 x;}}");

    puts("ok");
    return true;
}
