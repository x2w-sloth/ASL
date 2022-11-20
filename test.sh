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
        echo "$expect => $output"
    else
        echo "$input => $output, expected $expect"
        exit 1
    fi
}

if [[ ! -f "aslc" ]]; then
    make clean aslc
fi

assert 0   "{return 0;}"
assert 4   "{return 4;}"
assert 8   "{4; return 8;}"
assert 12  "{4;6; return 12;}"
assert 30  "{return 30; return 31;}"
assert 3   "{return 1+2;}"
assert 6   "{return 1+2 + 3;}"
assert 10  "{return 30-20;}"
assert 33  "{return 30 - 3 + 6;}"
assert 20  "{return 4+4*4;}"
assert 8   "{return 16*2/4;}"
assert 9   "{return 3*6/2;}"
assert 4   "{return (1+1)*2;}"
assert 10  "{return (2+3) * ((11-1)/5);}"
assert 4   "{return +5 + -1;}"
assert 30  "{return - - -30 + - -60;}"
assert 6   "{return (-8 + 5)*(- -2 - 4);}"
assert 2   "{;; return 2;}"
assert 1   "{{{}} return 1;}"
assert 3   "{1; {return 3;} return 2;}"
assert 0   "{return 0==1;}"
assert 1   "{return 42==42;}"
assert 1   "{return 0!=1;}"
assert 0   "{return 2!=2;}"
assert 1   "{return 0<1;}"
assert 0   "{return 1<1;}"
assert 0   "{return 2<1;}"
assert 1   "{return 0<=1;}"
assert 1   "{return 1<=1;}"
assert 0   "{return 2<=1;}"
assert 1   "{return 1>0;}"
assert 0   "{return 1>1;}"
assert 0   "{return 1>2;}"
assert 1   "{return 1>=0;}"
assert 1   "{return 1>=1;}"
assert 0   "{return 1>=2;}"
assert 1   "{return (1+2)*3 == 18/2;}"

echo "ok"
