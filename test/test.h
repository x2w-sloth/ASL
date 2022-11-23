#ifndef TEST_H
#define TEST_H

#define COUNT(A)        (sizeof(A) / sizeof(*(A)))
#define TEST(VAL, SRC)  if (!str_test(VAL, SRC)) return false

#include <stdio.h>
#include <stdbool.h>

// test modules
bool test_cmp();
bool test_expr();
bool test_locals();
bool test_pointer();
bool test_function();

// test.c
bool str_test(int val, const char *src);

#endif // TEST_H
