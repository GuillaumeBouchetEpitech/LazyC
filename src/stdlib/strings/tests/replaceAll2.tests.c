
#include "replaceAll2.tests.h"

#include "../replaceAll2.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

//
//
//

//
//
//

static void replaceAll2__can_replace_all()
{
  StringData result = String__replaceAll2("111-222-333-444-555-666-777-888-999", "666", "777");

  assert(strcmp(result.data, "111-222-333-444-555-777-777-888-999") == 0);
  assert(result.len == strlen("111-222-333-444-555-777-777-888-999"));

  free(result.data);
}

static void replaceAll2__can_replace_all_with_smaller_replacement()
{
  StringData result = String__replaceAll2("1111-2222-3333", "2222", "22");

  assert(strcmp(result.data, "1111-22-3333") == 0);
  assert(result.len == strlen("1111-22-3333"));

  free(result.data);
}

static void replaceAll2__can_replace_all_with_larger_replacement()
{
  StringData result = String__replaceAll2("1111-2222-3333", "2222", "22222222");

  assert(strcmp(result.data, "1111-22222222-3333") == 0);
  assert(result.len == strlen("1111-22222222-3333"));

  free(result.data);
}

//
//
//

//
//
//

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
  do { \
    printf("  TEST: %s ... ", #name); \
    tests_run++; \
  } while(0)

#define PASS() \
  do { \
    tests_passed++; \
    printf("PASS\n"); \
  } while(0)

#define RUN_TEST(test_func) \
  do { \
    TEST(test_func); \
    test_func(); \
    PASS(); \
  } while(0)

void String__replaceAll2_tests()
{
  printf("=== String__replaceAll2_tests ===\n\n");

  RUN_TEST(replaceAll2__can_replace_all);
  RUN_TEST(replaceAll2__can_replace_all_with_smaller_replacement);
  RUN_TEST(replaceAll2__can_replace_all_with_larger_replacement);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}

