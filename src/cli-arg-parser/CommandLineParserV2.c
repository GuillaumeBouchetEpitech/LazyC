
#include "./CommandLineParserV2.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

static void usage(const char *prog) {
  fprintf(stderr, "usage: %s --output-dir=FOLDER --base-dir=FOLDER --input-file=FILE\n", prog);
}

CommandLineOptsV2* CommandLineParserV2__parseArgs(int argc, char *const*argv)
{
  CommandLineOptsV2* newOpts = calloc(1, sizeof(CommandLineOptsV2));
  newOpts->includePath = PointerHeapArray__preAllocate(32);
  newOpts->libraryPath = PointerHeapArray__preAllocate(32);

  for (int ii = 1 ; ii < argc; ++ii)
  {
    const char* currArg = argv[ii];

    // printf(" ---> currArg: %s\n", currArg);

    char *delim = strchr(currArg, '=');

    // printf("   -> delim: %s\n", delim);

    if (delim == NULL) {
      printf("   -> DELIM NOT FOUND\n");
      usage(argv[0]);
      CommandLineParserV2__free(&newOpts);
      return NULL;
    }

    const int toDelim = (int)(delim - currArg);

    // printf("   -> toDelim: %d\n", toDelim);

    if (toDelim == 0) {
      printf("   -> EMPTY DELIM\n");
      usage(argv[0]);
      CommandLineParserV2__free(&newOpts);
      return NULL;
    }

    // char* key = strndup(currArg, toDelim);
    // printf("   -> key: %s\n", key);
    // free(key);

    // int valueLen = strlen(currArg) - (toDelim + 1);
    // printf("   -> value: %s\n", delim + 1);
    // // printf("   -> valueLen: %d\n", valueLen);

    if (strncmp(currArg, "--base-dir", toDelim) == 0)
    {
      // printf("   ---> baseDir SET\n");
      newOpts->baseDir = delim + 1;
    }
    else if (strncmp(currArg, "--input-file", toDelim) == 0)
    {
      // printf("   ---> entryFilepath SET\n");
      newOpts->entryFilepath = delim + 1;
    }
    else if (strncmp(currArg, "--output-dir", toDelim) == 0)
    {
      // printf("   ---> outputDir SET\n");
      newOpts->outputDir = delim + 1;
    }
    else if (strncmp(currArg, "--add-include-path", toDelim) == 0)
    {
      PointerHeapArray__pushBack(newOpts->includePath, delim + 1);
    }
    else if (strncmp(currArg, "--add-library-path", toDelim) == 0)
    {
      PointerHeapArray__pushBack(newOpts->libraryPath, delim + 1);
    }
    else
    {
      printf("   -> unknown arg: %s\n", currArg);
      usage(argv[0]);
      CommandLineParserV2__free(&newOpts);
      return NULL;
    }

  }


  if (
    !newOpts->outputDir ||
    !newOpts->baseDir ||
    !newOpts->entryFilepath
  ) {
    fprintf(stderr, "missing args\n");
    usage(argv[0]);
    return NULL;
  }

  // printf(" ==> newOpts->baseDir=%s", newOpts->baseDir);

  return newOpts;
}

void CommandLineParserV2__free(CommandLineOptsV2** self)
{
  PointerHeapArray__free(&(*self)->includePath);
  PointerHeapArray__free(&(*self)->libraryPath);
  free(*self);
  *self = NULL;
}

