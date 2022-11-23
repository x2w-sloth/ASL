#!/bin/bash

assert() {
    expect="$1"
    input="$2"

    ./aslc "$input" > tmp.s
    as tmp.s -o tmp.o -msyntax=intel -mnaked-reg
    ld tmp.o -o tmp
    ./tmp
    output="$?"

    if [ "$expect" = "$output" ]; then
        echo "$input => $output"
    else
        echo "$input => $output, expected $expect"
        exit 1
    fi
}

if [[ ! -f "aslc" ]]; then
    make clean aslc
fi

assert 0   "fn main() i64 {return 0;}"
assert 4   "fn main() i64 {return 4;}"
assert 8   "fn main() i64 {4; return 8;}"
assert 12  "fn main() i64 {4;6; return 12;}"
assert 30  "fn main() i64 {return 30; return 31;}"
assert 3   "fn main() i64 {return 1+2;}"
assert 6   "fn main() i64 {return 1+2 + 3;}"
assert 10  "fn main() i64 {return 30-20;}"
assert 33  "fn main() i64 {return 30 - 3 + 6;}"
assert 20  "fn main() i64 {return 4+4*4;}"
assert 8   "fn main() i64 {return 16*2/4;}"
assert 9   "fn main() i64 {return 3*6/2;}"
assert 4   "fn main() i64 {return (1+1)*2;}"
assert 10  "fn main() i64 {return (2+3) * ((11-1)/5);}"
assert 4   "fn main() i64 {return +5 + -1;}"
assert 30  "fn main() i64 {return - - -30 + - -60;}"
assert 6   "fn main() i64 {return (-8 + 5)*(- -2 - 4);}"
assert 2   "fn main() i64 {;; return 2;}"
assert 1   "fn main() i64 {{{}} return 1;}"
assert 3   "fn main() i64 {1; {return 3;} return 2;}"
assert 0   "fn main() i64 {return 0==1;}"
assert 1   "fn main() i64 {return 42==42;}"
assert 1   "fn main() i64 {return 0!=1;}"
assert 0   "fn main() i64 {return 2!=2;}"
assert 1   "fn main() i64 {return 0<1;}"
assert 0   "fn main() i64 {return 1<1;}"
assert 0   "fn main() i64 {return 2<1;}"
assert 1   "fn main() i64 {return 0<=1;}"
assert 1   "fn main() i64 {return 1<=1;}"
assert 0   "fn main() i64 {return 2<=1;}"
assert 1   "fn main() i64 {return 1>0;}"
assert 0   "fn main() i64 {return 1>1;}"
assert 0   "fn main() i64 {return 1>2;}"
assert 1   "fn main() i64 {return 1>=0;}"
assert 1   "fn main() i64 {return 1>=1;}"
assert 0   "fn main() i64 {return 1>=2;}"
assert 1   "fn main() i64 {return (1+2)*3 == 18/2;}"
assert 3   "fn main() i64 {i64 a=1+2; return a;}"
assert 2   "fn main() i64 {i64 a=3, z=5; return z-a;}"
assert 2   "fn main() i64 {i64 a1; i64 a2; a1=a2=1; return a1+a2;}"
assert 2   "fn main() i64 {i64 a=5,b=5; i64 c=5; return (a+b)/c;}"
assert 3   "fn main() i64 {i64 var; return var=3;}"
assert 4   "fn main() i64 { return get4(); } fn get4() i64 { return 2 * 2; }"
assert 10  "fn main() i64 { return get2() * get5(); } fn get2() i64 { return 2; } fn get5() i64 { return 5; }"
assert 10   "fn main() i64 { i64 x=10; return *&x; }"
assert 10   "fn main() i64 { i64 x=10; i64* y=&x; i64** z=&y; return **z; }"
assert 10   "fn main() i64 { i64 x=15; i64* y=&x; *y=10; return x; }"
assert 5    "fn main() i64 { i64 x=3; i64 y=5; return *(&x+1); }"
assert 3    "fn main() i64 { i64 x=3; i64 y=5; return *(&y-1); }"
assert 1    "fn main() i64 { i64 x=3; i64 y=1; return *(&x-(-y)); }"
assert 7    "fn main() i64 { i64 x, y=5; *(&x+1)=7; return y; }"
assert 7    "fn main() i64 { i64 x=5, y; *(&y-1)=7; return x; }"
assert 5    "fn main() i64 { i64 x; return (&x+3)-(&x-2); }"
assert 10   "fn main() i64 { i64 x; return &x + 1 - &x + 9; }"
assert 1    "fn main() i64 { return sub(2, 1); } fn sub(i64 a, i64 b) i64 { return a - b; }"
assert 2    "fn main() i64 { return sub(3,sub(2, 1)); } fn sub(i64 a, i64 b) i64 { return a - b; }"
assert 5    "fn main() i64 { return add3(2); } fn add3(i64 a) i64 { i64 b=3; return a+b; }"
assert 21   "fn main() i64 { return sum(1,2,3,4,5,6); } fn sum(i64 a, i64 b, i64 c, i64 d, i64 e, i64 f) { return a+b+c+d+e+f; }"
assert 4    "fn main() i64 { i64 x=3; foo(&x); return x; } fn foo(i64* x) { *x=4; }"

echo "ok"
