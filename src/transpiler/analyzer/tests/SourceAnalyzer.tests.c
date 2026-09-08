
#include "../SourceAnalyzer.h"

#include "stdlib/filesystem/pathUtils.h"
#include "stdlib/collections/PointerHeapArray.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>




static void SourceAnalyzer_can_be_created_and_freed() {
  SourceAnalyzer* newAnalyzer = SourceAnalyzer__create();
  assert(newAnalyzer != NULL);
  SourceAnalyzer__free(&newAnalyzer);
  assert(newAnalyzer == NULL);
}

static void _analyzeProject(unsigned int testFolderLen, const char* inSourceFilepath)
{
  SourceAnalyzer* newAnalyzer = SourceAnalyzer__create();
  assert(newAnalyzer != NULL);

  printf(" -> inSourceFilepath: %s\n", inSourceFilepath);

  {
    PointerHeapArray* includePaths = PointerHeapArray__preAllocate(32);

    SourceAnalyzer__scanFromMainFile(newAnalyzer, inSourceFilepath, includePaths);

    PointerHeapArray__free(&includePaths);

    unsigned int totalAnalyzed;
    char** allAnalyzed = SourceAnalyzer__getAllAnalyzed(newAnalyzer, &totalAnalyzed);

    printf(" -> totalAnalyzed:%d\n", totalAnalyzed);

    // assert(totalAnalyzed == 3);

    {
      const PointerHeapArray* sortedAnalyzedFiles = SourceAnalyzer__getSortedAnalyzedFiles(newAnalyzer);

      char buffer[1024];
      memset(buffer, 0, 1024);
      snprintf(buffer, 1024, "%s.actual-sorted-dep", inSourceFilepath);
      StreamWriter* streamWriter = StreamWriter__create(buffer);

      {
        snprintf(buffer, 1024, "total-files: %d\n", sortedAnalyzedFiles->len);
        StreamWriter__write(streamWriter, buffer, strlen(buffer));
      }

      for (unsigned int ii = 0; ii < sortedAnalyzedFiles->len; ++ii)
      {
        const AnalyzedFile* currFile = sortedAnalyzedFiles->data[ii];

        snprintf(buffer, 1024, "-[%d]-> %s\n", ii, currFile->filepath + testFolderLen);
        StreamWriter__write(streamWriter, buffer, strlen(buffer));

        unsigned int totalImports = 0;
        char** allImports = SourceIndexer__getAllImports(currFile->indexer, &totalImports);
        for (unsigned int jj = 0; jj < totalImports; ++jj)
        {
          snprintf(buffer, 1024, "  -[%d]-> %s\n", jj, allImports[jj] + testFolderLen);
          StreamWriter__write(streamWriter, buffer, strlen(buffer));
        }

        free(allImports);
      }

      StreamWriter__free(&streamWriter);
      // PointerHeapArray__free(&sortedAnalyzedFiles);
    }

    for (unsigned int ii = 0; ii < totalAnalyzed; ++ii)
    {
      printf("   -> allAnalyzed[%d]: %s\n", ii, allAnalyzed[ii]);

      const AnalyzedFile *currAnalyzed = SourceAnalyzer__getAnalyzed(newAnalyzer, allAnalyzed[ii]);

      {
        // debug
        char buffer[1024];
        memset(buffer, 0, 1024);
        snprintf(buffer, 1024, "%s.tree.debug", allAnalyzed[ii]);
        StreamWriter* streamWriter = StreamWriter__create(buffer);
        AnalyzedFile__debugTree(currAnalyzed, streamWriter);
        StreamWriter__free(&streamWriter);
      }

      {
        // debug
        char buffer[1024];
        memset(buffer, 0, 1024);
        snprintf(buffer, 1024, "%s.scope.debug", allAnalyzed[ii]);
        StreamWriter* streamWriter = StreamWriter__create(buffer);
        AnalyzedFile__debugScopeTree(currAnalyzed, streamWriter);
        StreamWriter__free(&streamWriter);
      }

    }

    free(allAnalyzed);
  }

  SourceAnalyzer__free(&newAnalyzer);
  assert(newAnalyzer == NULL);
}

static void SourceAnalyzer_can_scan_folder_from_main_file() {

  char* testFolder = Path__dirname(__FILE__);
  unsigned int testFolderLen = strlen(testFolder);

  {
    char* sourceFilepath = Path__join(2, testFolder, "tests-assets/simple/main.c");
    _analyzeProject(testFolderLen, sourceFilepath);
    free(sourceFilepath);
  }

  {
    char* sourceFilepath = Path__join(2, testFolder, "tests-assets/comptime-comments/main.lc");
    _analyzeProject(testFolderLen, sourceFilepath);
    free(sourceFilepath);
  }

  {
    char* sourceFilepath = Path__join(2, testFolder, "tests-assets/many-includes/main.lc");
    _analyzeProject(testFolderLen, sourceFilepath);
    free(sourceFilepath);
  }

  free(testFolder);
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

void SourceAnalyzer_tests()
{
  printf("=== SourceAnalyzer_tests ===\n\n");

  RUN_TEST(SourceAnalyzer_can_be_created_and_freed);
  RUN_TEST(SourceAnalyzer_can_scan_folder_from_main_file);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}


