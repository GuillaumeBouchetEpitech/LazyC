
#include "../executeCommand.h"

#include "./executeCommand.tests.h"

#include "stdlib/filesystem/pathUtils.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>



static void executeCommand_can_run_a_simple_list_command() {

  char* currentFolder = Path__dirname(__FILE__);
  printf("currentFolder %s\n", currentFolder);

  char *const argv[] = { "/bin/ls", "-lah", NULL };

  const int status = executeCommand("/bin/ls", argv, currentFolder);
  assert(status == 0);

  free(currentFolder);
}



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

void executeCommand_tests()
{
  printf("=== executeCommand_tests ===\n\n");

  RUN_TEST(executeCommand_can_run_a_simple_list_command);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}


