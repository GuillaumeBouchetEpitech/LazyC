
#include "../SourceParser.h"

#include "stdlib/filesystem/readFile.h"
#include "stdlib/filesystem/pathUtils.h"

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

static void SourceParser_can_be_created_and_freed() {
  SourceParser* newParser = SourceParser__create();
  assert(newParser != NULL);
  SourceParser__free(&newParser);
  assert(newParser == NULL);
}

static void SourceParser_can_parse_a_C_file() {
  SourceParser* newParser = SourceParser__create();
  assert(newParser != NULL);

  char* testFolder = Path__dirname(__FILE__);
  char* sourceFilepath = Path__join(2, testFolder, "tests-assets/simple/main.c");

  // SourceParsedFile* parsedFile = SourceParser__parse(newParser, "/home/barbie/Documents/_PROGRAMMING/tests/Typescript/OTHERS/transpiler-test/transpiler-c/assets/samples/small-sample/src/main.c");
  SourceParsedFile* parsedFile = SourceParser__parse(newParser, sourceFilepath);
  assert(parsedFile != NULL);
  SourceParsedFile__free(&parsedFile);
  assert(parsedFile == NULL);

  free(sourceFilepath);
  free(testFolder);

  SourceParser__free(&newParser);
  assert(newParser == NULL);
}

//MARK: _testOneFolder
static void _testOneFolder(const char* inTestFilepath) {

  // printf("testing: %s\n", inTestFilepath);

  char* currentTestFolder = Path__dirname(inTestFilepath);
  char* queryFilepath = Path__join(2, currentTestFolder, "test-query.txt");
  // char* expectedMatchesFilepath = Path__join(2, currentTestFolder, "main.c.expected-matches");
  // char* debugMatchesFilepath = Path__join(2, currentTestFolder, "main.c.actual-matches");

  char expectedMatchesFilepath[1024];
  char debugMatchesFilepath[1024];
  snprintf(expectedMatchesFilepath, 1024, "%s.expected-matches", inTestFilepath);
  snprintf(debugMatchesFilepath, 1024, "%s.actual-matches", inTestFilepath);

  SourceParser* newParser = SourceParser__create();
  assert(newParser != NULL);

  SourceParsedFile* parsedFile = SourceParser__parse(newParser, inTestFilepath);
  assert(parsedFile != NULL);

  const char* parsedFileContent = SourceParsedFile__getFileContent(parsedFile);

  {
    // debug
    char buffer[1024];
    memset(buffer, 0, 1024);
    snprintf(buffer, 1024, "%s.test.tree", inTestFilepath);
    StreamWriter* streamWriter = StreamWriter__create(buffer);
    SourceParsedFile__debugTree(parsedFile, streamWriter);
    StreamWriter__free(&streamWriter);
  }

  {
    StringData queryData;
    assert(readFile(queryFilepath, &queryData.data, &queryData.len) >= 0);

    StreamWriter* streamWriter = StreamWriter__create(debugMatchesFilepath);
    QueryMatchData* newMatchData = SourceParsedFile__query(parsedFile, queryData.data, queryData.len);

    char buffer[1024];
    memset(buffer, 0, 1024);

    QueryMatchData* cursor = newMatchData;
    while (cursor) {

      snprintf(buffer, 1024, "[MATCH:START]\n");
      StreamWriter__write(streamWriter, buffer, strlen(buffer));

      snprintf(buffer, 1024, "  totalNodes=%d\n", cursor->totalNodes);
      StreamWriter__write(streamWriter, buffer, strlen(buffer));

      for (unsigned int ii = 0; ii < cursor->totalNodes; ++ii)
      {
        NodeData* currNode = &cursor->allNodes[ii];

        snprintf(buffer, 1024, "    node[%d]: \"%s\"\n", ii, currNode->captureName.data);
        StreamWriter__write(streamWriter, buffer, strlen(buffer));

        snprintf(buffer, 1024, "      start: (%d) L%d_C%d\n", currNode->startPos.index, currNode->startPos.row, currNode->startPos.column);
        StreamWriter__write(streamWriter, buffer, strlen(buffer));

        snprintf(buffer, 1024, "      end:   (%d) L%d_C%d\n", currNode->endPos.index, currNode->endPos.row, currNode->endPos.column);
        StreamWriter__write(streamWriter, buffer, strlen(buffer));

        snprintf(buffer, 1024, "      [CONTENT]%.*s[/CONTENT]\n", (int)(currNode->endPos.index - currNode->startPos.index), parsedFileContent + currNode->startPos.index);
        StreamWriter__write(streamWriter, buffer, strlen(buffer));
      }

      snprintf(buffer, 1024, "[MATCH:END]\n");
      StreamWriter__write(streamWriter, buffer, strlen(buffer));

      cursor = cursor->next;
    }

    QueryMatchData__free(&newMatchData);
    StreamWriter__free(&streamWriter);

    free(queryData.data);
  }

  {
    StringData expectedData;
    assert(readFile(expectedMatchesFilepath, &expectedData.data, &expectedData.len) >= 0);

    StringData actualData;
    assert(readFile(debugMatchesFilepath, &actualData.data, &actualData.len) >= 0);

    assert(strcmp(actualData.data, expectedData.data) == 0);

    free(actualData.data);
    free(expectedData.data);
  }

  SourceParsedFile__free(&parsedFile);
  assert(parsedFile == NULL);

  SourceParser__free(&newParser);
  assert(newParser == NULL);

  // free(debugMatchesFilepath);
  // free(expectedMatchesFilepath);
  free(queryFilepath);
  free(currentTestFolder);
}

//MARK: simple
static void SourceParser_can_parse_a_C_file_and_query_match_it__simple() {
  char* currentFolder = Path__dirname(__FILE__);
  char* testsFolder = Path__join(2, currentFolder, "tests-assets/simple/main.c");
  _testOneFolder(testsFolder);
  free(testsFolder);
  free(currentFolder);
}

//MARK: simple
static void SourceParser_can_parse_a_C_file_and_query_match_it__complex() {
  char* currentFolder = Path__dirname(__FILE__);
  char* testsFolder = Path__join(2, currentFolder, "tests-assets/complex/main.c");
  _testOneFolder(testsFolder);
  free(testsFolder);
  free(currentFolder);
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

void SourceParser_tests()
{
  printf("=== SourceParser_tests ===\n\n");

  RUN_TEST(SourceParser_can_be_created_and_freed);
  RUN_TEST(SourceParser_can_parse_a_C_file);
  RUN_TEST(SourceParser_can_parse_a_C_file_and_query_match_it__simple);
  RUN_TEST(SourceParser_can_parse_a_C_file_and_query_match_it__complex);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}


