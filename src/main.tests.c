
#include "./stdlib/collections/tests/HeapArray.tests.lc"
#include "./stdlib/collections/tests/HashMap.tests.h"
#include "./stdlib/collections/tests/HashSet.tests.h"
#include "./stdlib/collections/tests/PointerHeapArray.tests.h"
#include "./stdlib/filesystem/tests/pathUtils.tests.h"
#include "./cli-arg-parser/tests/CommandLineParserV2.tests.h"
#include "./build-system/tests/BuildCmakeConfig.tests.h"
#include "./stdlib/sub-process/tests/executeCommand.tests.h"

#include "./stdlib/strings/tests/replaceAll2.tests.h"

#include "./transpiler/analyzer/internals/indexer/internals/tests/ComptimeCallRef.tests.h"
// #include "./transpiler/analyzer/internals/parser/tests/SourceParser.tests.h"
#include "./transpiler/analyzer/tests/SourceAnalyzer.tests.h"
#include "./transpiler/analyzer/internals/tests/AnalyzedFile.tests.h"
#include "./transpiler/tests/Transpiler.tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(void)
{
  printf("\n");
  printf("###############\n");
  printf("# TEST: START #\n");
  printf("###############\n");
  printf("\n");

  HeapArray_tests();
  HashMap_tests();
  HashSet_tests();
  PointerHeapArray_test();
  Path_tests();
  CommandLineParserV2_tests();
  BuildCmakeConfig_tests();
  executeCommand_tests();
  String__replaceAll2_tests();

  ComptimeCallRef_tests();
  // SourceParser_tests();
  SourceAnalyzer_tests();
  Transpiler_tests();
  AnalyzedFile_tests();

  printf("\n");
  printf("#############\n");
  printf("# TEST: END #\n");
  printf("#############\n");
  printf("\n");

  return EXIT_SUCCESS;
}
