
#include "../Transpiler.h"

#include "./Transpiler.tests.h"

#include "stdlib/filesystem/pathUtils.h"

#include "stdlib/collections/PointerHeapArray.h"

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

//
//
//

static void Transpiler_can_transpile_a_project()
{

  char *currentFolder = Path__dirname(__FILE__);
  char *tmpBaseDir = Path__join(2, currentFolder, "tests-assets/simple/src");
  char *tmpEntryFilepath = Path__join(2, tmpBaseDir, "main.lc");
  char *tmpOutputDir = Path__join(2, currentFolder, "tests-assets/simple/output");

  PointerHeapArray* includePaths = PointerHeapArray__preAllocate(32);

  // PointerHeapArray* sourcesFilepaths = PointerHeapArray__preAllocate(32);
  // PointerHeapArray__pushBack(sourcesFilepaths, strdup("main.c"));
  // PointerHeapArray__pushBack(sourcesFilepaths, strdup("sub-folder/sub-file.c"));
  // PointerHeapArray__pushBack(sourcesFilepaths, strdup("sub-folder/sub-file.h"));

  Transpiler* transpiler = Transpiler__create(tmpBaseDir, tmpEntryFilepath, tmpOutputDir, includePaths);
  assert(transpiler != NULL);

  PointerHeapArray__free(&includePaths);

  // const int result = Transpiler__applyDebug(transpiler);
  // assert(result == 0);

  Transpiler__free(&transpiler);
  assert(transpiler == NULL);

  // for (int ii = 0; ii < sourcesFilepaths->len; ++ii) {
  //   char* tmpStr = sourcesFilepaths->data[ii];
  //   free(tmpStr);
  // }
  // PointerHeapArray__free(&sourcesFilepaths);
  free(tmpOutputDir);
  free(tmpEntryFilepath);
  free(tmpBaseDir);
  free(currentFolder);
}

//
//
//

//
//
//

//
//
//

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name)                      \
  do                                    \
  {                                     \
    printf("  TEST: %s ... \n", #name); \
    tests_run++;                        \
  } while (0)

#define PASS(name)                        \
  do                                      \
  {                                       \
    tests_passed++;                       \
    printf("  TEST: %s PASS \n", #name);  \
  } while (0)

#define RUN_TEST(test_func) \
  do                        \
  {                         \
    TEST(test_func);        \
    test_func();            \
    PASS(test_func);        \
  } while (0)

void Transpiler_tests()
{
  printf("=== Transpiler_tests ===\n\n");

  RUN_TEST(Transpiler_can_transpile_a_project);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}
