#ifndef TEST_H
#define TEST_H

#define COUNT(A)              (sizeof(A) / sizeof(*(A)))
#define TEST(VAL, SRC)        if (!str_test(VAL, SRC))   return false
#define TEST_FILE(VAL, PATH)  if (!file_test(VAL, PATH)) return false

#include <stdio.h>
#include <stdbool.h>

// test modules
bool test_cmp();
bool test_expr();
bool test_flow();
bool test_locals();
bool test_globals();
bool test_scopes();
bool test_pointer();
bool test_function();
bool test_integration();

// test.c
bool str_test(int val, const char *src);
bool file_test(int val, const char *path);

#endif // TEST_H
