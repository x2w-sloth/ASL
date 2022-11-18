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

assert 0   "0"
assert 4   "4"
assert 30  "30"
assert 255 "255"
assert 3   "1+2"
assert 6   "1+2 + 3"
assert 10  "30-20"
assert 33  "30 - 3 + 6"
assert 20  "4+4*4"
assert 8   "16*2/4"
assert 9   "3*6/2"
assert 4   "(1+1)*2"
assert 10  "(2+3) * ((11-1)/5)"

echo "ok"
