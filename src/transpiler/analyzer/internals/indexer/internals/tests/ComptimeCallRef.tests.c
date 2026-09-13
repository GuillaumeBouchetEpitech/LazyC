
#include "../ComptimeCallRef.h"

#include "stdlib/collections/PointerHeapArray.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>




static void ComptimeCallRef_can_be_created_and_freed() {
  NodePos startPos = { 1,1,1 };
  NodePos endPos = { 10,10,10 };
  ComptimeCallRef newComptimeCallRef = ComptimeCallRef__create(startPos, endPos, "varName", "< int, Vec3 , LOL>");
  ComptimeCallRef__free(&newComptimeCallRef);
}

static void ComptimeCallRef_can_list_the_comptime_args() {
  NodePos startPos = { 1,1,1 };
  NodePos endPos = { 10,10,10 };
  ComptimeCallRef newComptimeCallRef = ComptimeCallRef__create(startPos, endPos, "varName", "< int, Vec3* , LOL>");

  assert(strcmp(newComptimeCallRef.varName, "varName") == 0);
  assert(newComptimeCallRef.argsList.len == 3);

  const char* argStr0 = newComptimeCallRef.argsList.data[0].data;
  assert(strcmp(argStr0, "int") == 0);

  const char* argStr1 = newComptimeCallRef.argsList.data[1].data;
  assert(strcmp(argStr1, "Vec3*") == 0);

  const char* argStr2 = newComptimeCallRef.argsList.data[2].data;
  assert(strcmp(argStr2, "LOL") == 0);

  assert(strcmp(newComptimeCallRef.signature, "varName__intVec3PtrLOL") == 0);

  ComptimeCallRef__free(&newComptimeCallRef);
}

static void ComptimeCallRef_can_support_comptime_args_that_also_comptime_types() {
  NodePos startPos = { 1,1,1 };
  NodePos endPos = { 10,10,10 };
  ComptimeCallRef newComptimeCallRef = ComptimeCallRef__create(startPos, endPos, "varName", "< int, subVarName<Vec3> , LOL>");

  assert(strcmp(newComptimeCallRef.varName, "varName") == 0);
  assert(newComptimeCallRef.argsList.len == 3);

  const char* argStr0 = newComptimeCallRef.argsList.data[0].data;
  assert(strcmp(argStr0, "int") == 0);

  const char* argStr1 = newComptimeCallRef.argsList.data[1].data;
  assert(strcmp(argStr1, "subVarName__Vec3") == 0);

  const char* argStr2 = newComptimeCallRef.argsList.data[2].data;
  assert(strcmp(argStr2, "LOL") == 0);

  assert(strcmp(newComptimeCallRef.signature, "varName__intsubVarName__Vec3LOL") == 0);

  ComptimeCallRef__free(&newComptimeCallRef);
}




static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
  do { \
    printf("  TEST: %s ...\n", #name); \
    tests_run++; \
  } while(0)

#define PASS(name) \
  do { \
    tests_passed++; \
    printf("  TEST: %s ... PASS\n", #name); \
  } while(0)

#define RUN_TEST(test_func) \
  do { \
    TEST(test_func); \
    test_func(); \
    PASS(test_func); \
  } while(0)

void ComptimeCallRef_tests()
{
  printf("=== ComptimeCallRef_tests ===\n\n");

  RUN_TEST(ComptimeCallRef_can_be_created_and_freed);
  RUN_TEST(ComptimeCallRef_can_list_the_comptime_args);
  RUN_TEST(ComptimeCallRef_can_support_comptime_args_that_also_comptime_types);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}


