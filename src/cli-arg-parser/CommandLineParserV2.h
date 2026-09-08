
#pragma once

#include "stdlib/collections/PointerHeapArray.h"

// TODO: in it's own heap allocated struct?
typedef struct CommandLineOptsV2 {
  char* baseDir;
  char* entryFilepath;
  char* outputDir;
  PointerHeapArray* includePath;
  PointerHeapArray* libraryPath;
} CommandLineOptsV2;


CommandLineOptsV2* CommandLineParserV2__parseArgs(int argc, char *const* argv);
void CommandLineParserV2__free(CommandLineOptsV2** self);
