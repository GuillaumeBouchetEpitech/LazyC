
#pragma once

#include "stdlib/collections/PointerHeapArray.h"

typedef struct BuildCmakeConfigOpts {
    const char *inBaseDir;
    PointerHeapArray *inSourcesFilepaths;
    const char *inOutputDir;
    PointerHeapArray *inIncludePath;
    PointerHeapArray *inLibraryPath;
    int doBuild;
} BuildCmakeConfigOpts;

int BuildCmakeConfig__generateCmakeFile(const BuildCmakeConfigOpts *opts);
