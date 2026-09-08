
#include "../CommandLineParserV2.h"

#include "stdlib/filesystem/pathUtils.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>




static void CommandLineParserV2_can_parse_command_line_args() {

  const char* tmpArgv[4] = {
    "./bin/exec",
    "--base-dir=/lol/omg",
    "--input-file=/lol/omg/main.c",
    "--output-dir=/lol/test"
  };

  CommandLineOptsV2* opts = CommandLineParserV2__parseArgs(4, tmpArgv);

  assert(opts != NULL);
  assert(strcmp(opts->baseDir, "/lol/omg") == 0);
  assert(strcmp(opts->entryFilepath, "/lol/omg/main.c") == 0);
  assert(strcmp(opts->outputDir, "/lol/test") == 0);
  assert(opts->includePath->len == 0);
  assert(opts->libraryPath->len == 0);

  CommandLineParserV2__free(&opts);
}

static void CommandLineParserV2_can_parse_command_line_args_with_includepath_and_librarypath() {

  const char* tmpArgv[6] = {
    "./bin/exec",
    "--base-dir=/lol/omg",
    "--input-file=/lol/omg/main.c",
    "--output-dir=/lol/test",
    "--add-include-path=/lol/include",
    "--add-library-path=/lol/library"
  };

  CommandLineOptsV2* opts = CommandLineParserV2__parseArgs(6, tmpArgv);

  assert(opts != NULL);
  assert(strcmp(opts->baseDir, "/lol/omg") == 0);
  assert(strcmp(opts->entryFilepath, "/lol/omg/main.c") == 0);
  assert(strcmp(opts->outputDir, "/lol/test") == 0);
  assert(opts->includePath->len == 1);
  assert(strcmp(opts->includePath->data[0], "/lol/include") == 0);
  assert(opts->libraryPath->len == 1);
  assert(strcmp(opts->libraryPath->data[0], "/lol/library") == 0);

  CommandLineParserV2__free(&opts);
}




static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
  do { \
    printf("  TEST: %s ... \n", #name); \
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

void CommandLineParserV2_tests()
{
  printf("=== CommandLineParserV2_tests ===\n\n");

  RUN_TEST(CommandLineParserV2_can_parse_command_line_args);
  RUN_TEST(CommandLineParserV2_can_parse_command_line_args_with_includepath_and_librarypath);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}


