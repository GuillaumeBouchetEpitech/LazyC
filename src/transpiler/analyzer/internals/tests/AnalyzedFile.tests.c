
#include "AnalyzedFile.tests.h"

#include "../AnalyzedFile.h"
#include "../parser/SourceParser.h"

#include "stdlib/filesystem/readFile.h"
#include "stdlib/filesystem/pathUtils.h"
#include "stdlib/filesystem/statUtils.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>




static void AnalyzedFile_can_be_created_and_freed() {
  char* testFolder = Path__dirname(__FILE__);
  SourceParser* parser = SourceParser__create();

  {
    char* sourceFilepath = Path__join(2, testFolder, "tests-assets/simple.c");
    AnalyzedFile* newAnalyzed = AnalyzedFile__create(parser, sourceFilepath);
    assert(newAnalyzed != NULL);
    AnalyzedFile__free(&newAnalyzed);
    assert(newAnalyzed == NULL);
    free(sourceFilepath);
  }

  SourceParser__free(&parser);
  free(testFolder);
}



static void _analyzeProject(SourceParser* parser, const char* inBaseDir, const char* inSourceFilepath, int doCheck)
{
  char expectedMatchesFilepath[1024];
  char debugMatchesFilepath[1024];
  snprintf(expectedMatchesFilepath, 1024, "%s.expected-matches", inSourceFilepath);
  snprintf(debugMatchesFilepath, 1024, "%s.actual-matches", inSourceFilepath);


  AnalyzedFile* newAnalyzed = AnalyzedFile__create(parser, inSourceFilepath);
  assert(newAnalyzed != NULL);
  assert(strcmp(newAnalyzed->filepath, inSourceFilepath) == 0);

  char* extName = Path__extname(inSourceFilepath);
  if (strcmp(extName, ".c") == 0) {
    assert(newAnalyzed->fileType == SOURCE_C);
  }
  else if (strcmp(extName, ".h") == 0) {
    assert(newAnalyzed->fileType == SOURCE_H);
  }
  else if (strcmp(extName, ".lc") == 0) {
    assert(newAnalyzed->fileType == SOURCE_LC);
  }

  assert(newAnalyzed->parsedFile != NULL);
  assert(newAnalyzed->indexer != NULL);

  printf(" -> inSourceFilepath: %s\n", inSourceFilepath);

  {
    char buffer[1024];
    memset(buffer, 0, 1024);
    snprintf(buffer, 1024, "%s.test.tree.debug", inSourceFilepath);
    StreamWriter* streamWriter = StreamWriter__create(buffer);
    AnalyzedFile__debugTree(newAnalyzed, streamWriter);
    StreamWriter__free(&streamWriter);
  }

  {
    StreamWriter* streamWriter = StreamWriter__create(debugMatchesFilepath);
    AnalyzedFile__debugScopeTree(newAnalyzed, inBaseDir, streamWriter);
    StreamWriter__free(&streamWriter);
  }

  if (doCheck != 0)
  {

    // printf(" -> Stat__pathExist(expectedMatchesFilepath): %d\n", Stat__pathExist(expectedMatchesFilepath));
    // printf(" -> Stat__pathExist(debugMatchesFilepath):    %d\n", Stat__pathExist(debugMatchesFilepath));

    assert(Stat__pathExist(expectedMatchesFilepath) == 1);
    assert(Stat__pathExist(debugMatchesFilepath) == 1);

    StringData expectedData;
    assert(readFile(expectedMatchesFilepath, &expectedData.data, &expectedData.len) >= 0);
    assert(expectedData.data != NULL);
    assert(expectedData.len > 0);

    StringData actualData;
    assert(readFile(debugMatchesFilepath, &actualData.data, &actualData.len) >= 0);
    assert(actualData.data != NULL);
    assert(actualData.len > 0);

    assert(actualData.len == expectedData.len);
    assert(strcmp(actualData.data, expectedData.data) == 0);

    free(actualData.data);
    free(expectedData.data);
  }

  free(extName);
  AnalyzedFile__free(&newAnalyzed);
  assert(newAnalyzed == NULL);
}


static void AnalyzedFile_can_analyze_source_file__simple_C() {
  char* testFolder = Path__dirname(__FILE__);
  SourceParser* parser = SourceParser__create();

  {
    char* sourceFilepath = Path__join(2, testFolder, "tests-assets/simple.c");
    _analyzeProject(parser, testFolder, sourceFilepath, 1);
    free(sourceFilepath);
  }

  {
    char* sourceFilepath = Path__join(2, testFolder, "tests-assets/comptime-comments.lc");
    _analyzeProject(parser, testFolder, sourceFilepath, 1);
    free(sourceFilepath);
  }

  {
    char* sourceFilepath = Path__join(2, testFolder, "tests-assets/importing-files.lc");
    _analyzeProject(parser, testFolder, sourceFilepath, 1);
    free(sourceFilepath);
  }

  {
    char* sourceFilepath = Path__join(2, testFolder, "tests-assets/define-struct-union-enum-typedef.h");
    _analyzeProject(parser, testFolder, sourceFilepath, 0);
    free(sourceFilepath);
  }

  SourceParser__free(&parser);
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

void AnalyzedFile_tests()
{
  printf("=== AnalyzedFile_tests ===\n\n");

  RUN_TEST(AnalyzedFile_can_be_created_and_freed);
  RUN_TEST(AnalyzedFile_can_analyze_source_file__simple_C);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}


