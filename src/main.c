
// #include "stdlib/filesystem/readFile.h"
// #include "parser/SourceParser.h"

#include "./cli-arg-parser/CommandLineParserV2.h"
#include "./transpiler/analyzer/SourceAnalyzer.h"
#include "./transpiler/Transpiler.h"
#include "./build-system/BuildCmakeConfig.h"

#include "./stdlib/core/panic.h"
#include "./stdlib/collections/PointerHeapArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char **argv)
{
  CommandLineOptsV2* cmdOpts = CommandLineParserV2__parseArgs(argc, argv);
  if (cmdOpts == NULL) {
    return EXIT_FAILURE;
  }

  printf(" -> cmdOpts->outputDir:    %s\n", cmdOpts->outputDir);
  printf(" -> cmdOpts->baseDir:      %s\n", cmdOpts->baseDir);
  printf(" -> cmdOpts->entryFilepath:%s\n", cmdOpts->entryFilepath);

  Transpiler* transpiler = Transpiler__create(cmdOpts->baseDir, cmdOpts->entryFilepath, cmdOpts->outputDir, cmdOpts->includePath);
  if (!transpiler) {
    panic("could not create the transpiler");
  }

  // if (Transpiler__applyDebug(transpiler) != 0) {
  //   panic("could not debug the transpiler");
  // }

  // PointerHeapArray<cstring>
  PointerHeapArray* sourcesFilepaths = Transpiler__getOutputSource(transpiler);

  BuildCmakeConfigOpts buildOpts;
  buildOpts.inBaseDir = cmdOpts->baseDir;
  buildOpts.inSourcesFilepaths = sourcesFilepaths;
  buildOpts.inOutputDir = cmdOpts->outputDir;
  buildOpts.inIncludePath = cmdOpts->includePath;
  buildOpts.inLibraryPath = cmdOpts->libraryPath;
  buildOpts.doBuild = 1;

  if (BuildCmakeConfig__generateCmakeFile(&buildOpts) != 0) {
    panic("could generate a CMakeFile");
  }

  PointerHeapArray__free(&sourcesFilepaths);
  Transpiler__free(&transpiler);
  CommandLineParserV2__free(&cmdOpts);

  return EXIT_SUCCESS;
}