
#include "../BuildCmakeConfig.h"

#include "./BuildCmakeConfig.tests.h"

#include "stdlib/filesystem/pathUtils.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>



static void BuildCmakeConfig_can_generate_a_cmake_file() {


  char* currentFolder = Path__dirname(__FILE__);
  char *tmpBaseDir = Path__join(2, currentFolder, "tests-assets/simple/src");
  char* tmpOutputDir = Path__join(2, currentFolder, "tests-assets/simple/output");

  PointerHeapArray* sourcesFilepaths = PointerHeapArray__preAllocate(32);
  PointerHeapArray__pushBack(sourcesFilepaths, strdup("main.c"));
  PointerHeapArray__pushBack(sourcesFilepaths, strdup("sub-folder/sub-file.c"));
  PointerHeapArray__pushBack(sourcesFilepaths, strdup("sub-folder/sub-file.h"));

  BuildCmakeConfigOpts opts;
  opts.inBaseDir = tmpBaseDir;
  opts.inSourcesFilepaths = sourcesFilepaths;
  opts.inOutputDir = tmpOutputDir;
  opts.inIncludePath = NULL;
  opts.inLibraryPath = NULL;
  opts.doBuild = 0;
  int result = BuildCmakeConfig__generateCmakeFile(&opts);
  assert(result == 0);

  for (int ii = 0; ii < sourcesFilepaths->len; ++ii) {
    char* tmpStr = sourcesFilepaths->data[ii];
    free(tmpStr);
  }
  PointerHeapArray__free(&sourcesFilepaths);
  free(tmpOutputDir);
  free(tmpBaseDir);
  free(currentFolder);
}

static void BuildCmakeConfig_can_generate_a_cmake_file_includepath_and_librarypath() {


  char* currentFolder = Path__dirname(__FILE__);
  char *tmpBaseDir = Path__join(2, currentFolder, "tests-assets/simple2/src");
  char* tmpOutputDir = Path__join(2, currentFolder, "tests-assets/simple2/output");

  PointerHeapArray* sourcesFilepaths = PointerHeapArray__preAllocate(32);
  PointerHeapArray__pushBack(sourcesFilepaths, strdup("main.c"));
  PointerHeapArray__pushBack(sourcesFilepaths, strdup("sub-folder/sub-file.c"));
  PointerHeapArray__pushBack(sourcesFilepaths, strdup("sub-folder/sub-file.h"));

  PointerHeapArray* includeFilepaths = PointerHeapArray__preAllocate(32);
  PointerHeapArray__pushBack(includeFilepaths, strdup("tests-assets/simple/lib"));

  PointerHeapArray* libraryFilepaths = PointerHeapArray__preAllocate(32);
  PointerHeapArray__pushBack(libraryFilepaths, strdup("tests-assets/simple/lib"));

  BuildCmakeConfigOpts opts;
  opts.inBaseDir = tmpBaseDir;
  opts.inSourcesFilepaths = sourcesFilepaths;
  opts.inOutputDir = tmpOutputDir;
  opts.inIncludePath = includeFilepaths;
  opts.inLibraryPath = libraryFilepaths;
  opts.doBuild = 0;
  int result = BuildCmakeConfig__generateCmakeFile(&opts);
  assert(result == 0);

  for (int ii = 0; ii < sourcesFilepaths->len; ++ii) {
    char* tmpStr = sourcesFilepaths->data[ii];
    free(tmpStr);
  }
  PointerHeapArray__free(&sourcesFilepaths);
  for (int ii = 0; ii < includeFilepaths->len; ++ii) {
    char* tmpStr = includeFilepaths->data[ii];
    free(tmpStr);
  }
  PointerHeapArray__free(&includeFilepaths);
  for (int ii = 0; ii < libraryFilepaths->len; ++ii) {
    char* tmpStr = libraryFilepaths->data[ii];
    free(tmpStr);
  }
  PointerHeapArray__free(&libraryFilepaths);
  free(tmpOutputDir);
  free(tmpBaseDir);
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

void BuildCmakeConfig_tests()
{
  printf("=== BuildCmakeConfig_tests ===\n\n");

  RUN_TEST(BuildCmakeConfig_can_generate_a_cmake_file);
  RUN_TEST(BuildCmakeConfig_can_generate_a_cmake_file_includepath_and_librarypath);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}


