
#include "Transpiler.h"

#include "./analyzer/SourceAnalyzer.h"

#include "./analyzer/internals/indexer/internals/ComptimeCallRef.h"

#include "../build-system/BuildCmakeConfig.h"

#include "stdlib/collections/PointerHeapArray.h"
#include "stdlib/collections/HashSet.h"
#include "stdlib/filesystem/ensureFolder.h"
#include "stdlib/filesystem/pathUtils.h"
#include "stdlib/filesystem/StreamWriter.h"
#include "stdlib/filesystem/readFile.h"
#include "stdlib/filesystem/writeFile.h"
#include "stdlib/strings/replaceAll.h"
#include "stdlib/strings/replaceAll2.h"
#include "stdlib/strings/StringBuffer.h"
#include "stdlib/strings/trimStr.h"
#include "stdlib/sub-process/executeCommand.h"
#include "stdlib/time/StopWatch.h"
#include "stdlib/core/panic.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Transpiler {
  // analyzer
  // cmake config builder
  char* baseDir;
  char* entryFilepath;
  char* outputDir;
  int handleTests;
  SourceAnalyzer* analyzer;

  HashSet* alreadyProcessedFiles;
  HashSet* allGeneratedFiles;

  // HashMap<csring, HashSet>
  HashMap* allExtraIncludesPerFiles;

} Transpiler;

static int _Transpiler__processComptime(Transpiler* self, PointerHeapArray* inIncludePath);
static int _Transpiler__processFile(Transpiler* self, const AnalyzedFile* inAnalyzedFile);

//MARK: create
Transpiler* Transpiler__create(
  const char* inBaseDir,
  const char* inEntryFilepath,
  const char* inOutputDir,
  PointerHeapArray* inIncludePath,
  int handleTests)
{

  // (void)inBaseDir;
  // (void)inOutputDir;

  Transpiler *newTranspiler = calloc(1, sizeof(Transpiler));
  if (!newTranspiler)
  {
    return NULL;
  }

  newTranspiler->baseDir = strdup(inBaseDir);
  newTranspiler->entryFilepath = strdup(inEntryFilepath);
  newTranspiler->outputDir = strdup(inOutputDir);
  newTranspiler->handleTests = handleTests != 0 ? 1 : 0;

  PointerHeapArray* newIncludePath = PointerHeapArray__preAllocate(32);
  PointerHeapArray__pushBack(newIncludePath, newTranspiler->baseDir);
  for (unsigned int ii = 0; ii < inIncludePath->len; ++ii)
  {
    // char* currPath = inIncludePath->data[ii];
    PointerHeapArray__pushBack(newIncludePath, inIncludePath->data[ii]);
  }

  newTranspiler->analyzer = SourceAnalyzer__create(newTranspiler->handleTests);
  SourceAnalyzer__scanFromMainFile(newTranspiler->analyzer, newTranspiler->entryFilepath, newIncludePath);

  // // debug
  // {
  //   const PointerHeapArray * sortedAnalyzedFiles = SourceAnalyzer__getSortedAnalyzedFiles(newTranspiler->analyzer);
  //   printf(" -> totalAnalyzed:%d\n", sortedAnalyzedFiles->len);
  //   for (unsigned int ii = 0; ii < sortedAnalyzedFiles->len; ++ii)
  //   {
  //     const AnalyzedFile* analyzedFile = sortedAnalyzedFiles->data[ii];
  //     printf("   -> analyzed[%d]: %s\n", ii, analyzedFile->filepath);
  //   }
  //   Transpiler__applyDebug(newTranspiler);
  // }



  newTranspiler->alreadyProcessedFiles = HashSet__preAllocate(32);
  newTranspiler->allGeneratedFiles = HashSet__preAllocate(32);

  newTranspiler->allExtraIncludesPerFiles = HashMap__preAllocate(32);

  {
    if (_Transpiler__processComptime(newTranspiler, newIncludePath) < 0) {
      Transpiler__free(&newTranspiler);
      return NULL;
    }
  }

  PointerHeapArray__free(&newIncludePath);

  {
    const PointerHeapArray * sortedAnalyzedFiles = SourceAnalyzer__getSortedAnalyzedFiles(newTranspiler->analyzer);


    // printf(" -> totalAnalyzed:%d\n", sortedAnalyzedFiles->len);

    for (unsigned int ii = 0; ii < sortedAnalyzedFiles->len; ++ii)
    {
      const AnalyzedFile* analyzedFile = sortedAnalyzedFiles->data[ii];

      if (
        SourceIndexer__hasMainFunction(analyzedFile->indexer) == 1 &&
        strcmp(analyzedFile->filepath, newTranspiler->entryFilepath) != 0
      ) {
        // skip any other files with a main function
        continue;
      }

      _Transpiler__processFile(newTranspiler, analyzedFile);
    }

    // PointerHeapArray__free(&sortedAnalyzed);
    // free(allAnalyzed);
  }

  return newTranspiler;
}

//MARK: free
void Transpiler__free(Transpiler** self)
{
  if (!self || !*self) { return; }

  {
    unsigned int totalKeys = 0;
    EntryItem* allItems = HashMap__get_allItems((*self)->allExtraIncludesPerFiles, &totalKeys);
    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      HashSet* tmpSet = allItems[ii].value;
      HashSet__free(&tmpSet);
    }
    free(allItems);
    HashMap__free(&(*self)->allExtraIncludesPerFiles);
  }

  HashSet__free(&(*self)->alreadyProcessedFiles);
  HashSet__free(&(*self)->allGeneratedFiles);

  SourceAnalyzer__free(&(*self)->analyzer);

  free((*self)->outputDir);
  free((*self)->entryFilepath);
  free((*self)->baseDir);

  free(*self);
  *self = NULL;
}




//MARK: applyDebug
int Transpiler__applyDebug(Transpiler* self)
{
  // unsigned int totalAnalyzed;
  // char** allAnalyzed = SourceAnalyzer__getAllAnalyzed(self->analyzer, &totalAnalyzed);
  // if (!allAnalyzed) {
  //   return -1;
  // }
  const PointerHeapArray * sortedAnalyzedFiles = SourceAnalyzer__getSortedAnalyzedFiles(self->analyzer);

  // printf(" -> totalAnalyzed:%d\n", sortedAnalyzedFiles->len);

  for (unsigned int ii = 0; ii < sortedAnalyzedFiles->len; ++ii)
  {
    const AnalyzedFile* currAnalyzed = sortedAnalyzedFiles->data[ii];

    {
      // debug
      char buffer[1024];
      memset(buffer, 0, 1024);
      snprintf(buffer, 1024, "%s.tree.debug", currAnalyzed->filepath);
      StreamWriter* streamWriter = StreamWriter__create(buffer);
      AnalyzedFile__debugTree(currAnalyzed, streamWriter);
      StreamWriter__free(&streamWriter);
    }

    {
      // debug
      char buffer[1024];
      memset(buffer, 0, 1024);
      snprintf(buffer, 1024, "%s.scope.debug", currAnalyzed->filepath);
      StreamWriter* streamWriter = StreamWriter__create(buffer);
      AnalyzedFile__debugScopeTree(currAnalyzed, self->baseDir, streamWriter);
      StreamWriter__free(&streamWriter);
    }

  }

  // free(allAnalyzed);
  return 0;
}



//MARK: _processComptime
static int _Transpiler__processComptime(Transpiler* self, PointerHeapArray* inIncludePath)
{
  // unsigned int totalAnalyzed;
  // char** allAnalyzed = SourceAnalyzer__getAllAnalyzed(self->analyzer, &totalAnalyzed);
  // if (!allAnalyzed) {
  //   return -1;
  // }

  // PointerHeapArray<AnalyzedFile>
  const PointerHeapArray * sortedAnalyzedFiles = SourceAnalyzer__getSortedAnalyzedFiles(self->analyzer);

  // determine if we even have any comptime calls
  int hasComptimeCalls = 0;
  for (unsigned int ii = 0; ii < sortedAnalyzedFiles->len; ++ii)
  {
    // const AnalyzedFile* analyzedFile = SourceAnalyzer__getAnalyzed(self->analyzer, allAnalyzed[ii]);
    const AnalyzedFile* analyzedFile = sortedAnalyzedFiles->data[ii];

    const SourceIndexer* indexer = AnalyzedFile__getIndexer(analyzedFile);

    const HeapArray<ComptimeCallRef>* comptimeCallsList = SourceIndexer__getComptimeCallsList(indexer);

    // if (SourceIndexer__hasComptimeCalls(indexer) != 0) {
    if (comptimeCallsList->len > 0) {
      // printf(" -> hasComptimeCalls -> %s\n", analyzedFile->filepath);
      hasComptimeCalls = 1;
      break;
    }
  }

  if (hasComptimeCalls == 0) {
    printf("HAS NO COMPTIME\n");
    // no comptime calls -> skip
    // free(allAnalyzed);
    return 0;
  }
  printf("HAS COMPTIME\n");

  printf("##\n");
  printf("####\n");
  printf("######\n");
  printf("########\n");

  // ensure the right `{baseDir}/.generated` folder
  // -> for the `comptime.main.c` file
  char* generatedFolderPath = Path__join(2, self->baseDir, ".generated");
  if (!generatedFolderPath || ensureFolder(generatedFolderPath) != 0)
  {
    panic("could not ensure the main .generated folder");
    // // TODO: need a goto to a failure_return
    // free(generatedFolderPath);
    // // free(allAnalyzed);
    // return -1;
  }

  // a hashmap of comptime-call-signatures
  // -> key: signature
  // -> value: {
  // ---> source file
  // ---> comptimeRef
  // -> }

  // find all the unique comptime calls
  HashMap* allUniqueComptimeCallMap = HashMap__preAllocate(32);
  HashSet* allUniqueComptimeNameSet = HashSet__preAllocate(32);
  // for (unsigned int ii = 0; ii < totalAnalyzed; ++ii)

  for (unsigned int ii = 0; ii < sortedAnalyzedFiles->len; ++ii)
  {
    // const AnalyzedFile* analyzedFile = SourceAnalyzer__getAnalyzed(self->analyzer, allAnalyzed[ii]);
    const AnalyzedFile* analyzedFile = sortedAnalyzedFiles->data[ii];

    const HeapArray<ComptimeCallRef>* comptimeCallsList = SourceIndexer__getComptimeCallsList(analyzedFile->indexer);

    for (unsigned int callIdx = 0; callIdx < comptimeCallsList->len; ++callIdx)
    {
      ComptimeCallRef* currComptime = &comptimeCallsList->data[callIdx];

      // printf(" -> callName: %s\n", currComptime->varName);
      // printf("   -> totalComptimeCallsArg: %d\n", currComptime->argsList.len);
      // for (unsigned int callArgIdx = 0; callArgIdx < currComptime->argsList.len; ++callArgIdx)
      // {
      //   const char* argStr = currComptime->argsList.data[callArgIdx];
      //   printf("     -> args[%d]: %s\n", callArgIdx, argStr);
      // }

      if (!HashMap__contains(allUniqueComptimeCallMap, currComptime->signature))
      {
        HashMap__set(allUniqueComptimeCallMap, currComptime->signature, currComptime);
        HashSet__set(allUniqueComptimeNameSet, currComptime->varName);
      }


      {
        // register the relationship between "comptime calling files" and the "comptime generated file"

        const AnalyzedFile* comptimeDefFile = SourceAnalyzer__getAnalyzedFromDefinitionName(self->analyzer, currComptime->varName);
        if (comptimeDefFile) {
          // printf(" -> %s => %s\n", currName, comptimeDefFile->filepath);
          // PointerHeapArray__pushBack(allFilesToInclude, comptimeDefFile->filepath);




          char buffer[1024];
          memset(buffer, 0, 1024);

          // const AnalyzedFile* currFile = SourceAnalyzer__getAnalyzedFromDefinitionName(self->analyzer, currComptime->varName);
          char* comptimeFuncDefFolderPath = Path__dirname(comptimeDefFile->filepath);
          char* comptimeFuncDefGeneratedFolderPath = Path__join(2, comptimeFuncDefFolderPath, ".generated");

          char* comptimeFuncDefFilename = Path__basename(comptimeDefFile->filepath);
          snprintf(buffer, 1024, "%s.%s.h", comptimeFuncDefFilename, currComptime->signature);

          char* absoluteFilepath = Path__join(2, comptimeFuncDefGeneratedFolderPath, buffer);




          // get or create HashSet
          HashSet* tmpSet = HashMap__get(self->allExtraIncludesPerFiles, analyzedFile->filepath);
          if (!tmpSet) {
            tmpSet = HashSet__preAllocate(32);
            HashMap__set(self->allExtraIncludesPerFiles, analyzedFile->filepath, tmpSet);
          }
          // add the relationship
          HashSet__set(tmpSet, absoluteFilepath);

          // printf(" => FOUND A GENERATED RELATIONSHIP %s -> %s\n", analyzedFile->filepath, absoluteFilepath);




          free(comptimeFuncDefFolderPath);
          free(comptimeFuncDefGeneratedFolderPath);
          free(comptimeFuncDefFilename);
          free(absoluteFilepath);
        }
      }

    }
  }

  // determine the `comptime calls` definition
  // -> their `comptime function definition`

  PointerHeapArray* allFilesToInclude = PointerHeapArray__preAllocate(32);

  {
    unsigned int totalKeys = 0;
    char** allKeys = HashSet__get_allKeys(allUniqueComptimeNameSet, &totalKeys);

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      const char* currName = allKeys[ii];

      const AnalyzedFile* currFile = SourceAnalyzer__getAnalyzedFromDefinitionName(self->analyzer, currName);
      if (currFile) {
        // printf(" -> %s => %s\n", currName, currFile->filepath);
        PointerHeapArray__pushBack(allFilesToInclude, currFile->filepath);
      }
    }

    free(allKeys);
  }


  // ensure the ".generated" folder at the comptime function definition location
  {
    for (unsigned ii = 0; ii < allFilesToInclude->len; ++ii)
    {
      const char* currFilepath = allFilesToInclude->data[ii];

      char* comptimeFuncDefFolderPath = Path__dirname(currFilepath);
      char* comptimeFuncDefGeneratedFolderPath = Path__join(2, comptimeFuncDefFolderPath, ".generated");

      if (ensureFolder(comptimeFuncDefGeneratedFolderPath) != 0)
      {
        panic("count not ensure a comptime .generated folder");
        // // TODO: need a goto to a failure_return
        // return -1;
      }

      free(comptimeFuncDefFolderPath);
      free(comptimeFuncDefGeneratedFolderPath);
    }
  }

  PointerHeapArray* allComptimeGeneratedFiles = PointerHeapArray__preAllocate(32);

  char* comptimeMainFilepath = Path__join(2, generatedFolderPath, "comptime.main.lc");

  // generate the main.c file
  {
    StreamWriter* streamWriter = StreamWriter__create(comptimeMainFilepath);

    char buffer[1024];
    memset(buffer, 0, 1024);

    for (unsigned ii = 0; ii < allFilesToInclude->len; ++ii)
    {
      const char* currFilepath = allFilesToInclude->data[ii];
      char* relativePath = Path__relative(generatedFolderPath, currFilepath);

      snprintf(buffer, 1024, "\n#include \"%s\"\n", relativePath);
      StreamWriter__write(streamWriter, buffer, strlen(buffer));

      free(relativePath);
    }

    {
      const char* k_func = "\n"
        "\n"
        "#include <fcntl.h>\n"
        "#include <stdio.h>\n"
        "#include <unistd.h>\n"
        "\n"
        "static int redirect_stdout(const char *path) {\n"
        // "    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);\n"
        "    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);\n"
        "    if (fd < 0) return -1;\n"
        "    // don't lose buffered bytes to the old file\n"
        "    fflush(stdout);\n"
        "    int r = dup2(fd, STDOUT_FILENO);\n"
        "    close(fd);\n"
        "    return r < 0 ? -1 : 0;\n"
        "}\n";

      StreamWriter__write(streamWriter, k_func, strlen(k_func));
    }

    {
      const char* k_func = "\n"
        "int main()\n"
        "{\n"
        "    printf(\"COMPTIME START!\\n\");\n"
        "    // keep the terminal\n"
        "    int saved = dup(STDOUT_FILENO);\n"
        "\n";

      StreamWriter__write(streamWriter, k_func, strlen(k_func));
    }

    {
      unsigned int totalKeys = 0;
      EntryItem* items = HashMap__get_allItems(allUniqueComptimeCallMap, &totalKeys);

      for (unsigned int ii = 0; ii < totalKeys; ++ii)
      {
        const ComptimeCallRef* currComptime = items[ii].value;

        // redirect stdout output to file

        {
          const AnalyzedFile* currFile = SourceAnalyzer__getAnalyzedFromDefinitionName(self->analyzer, currComptime->varName);
          char* comptimeFuncDefFolderPath = Path__dirname(currFile->filepath);
          char* comptimeFuncDefGeneratedFolderPath = Path__join(2, comptimeFuncDefFolderPath, ".generated");

          char* comptimeFuncDefFilename = Path__basename(currFile->filepath);
          snprintf(buffer, 1024, "%s.%s.lc", comptimeFuncDefFilename, currComptime->signature);

          char* absoluteFilepath = Path__join(2, comptimeFuncDefGeneratedFolderPath, buffer);
          // char* relativeFilepath = Path__relative(generatedFolderPath, absoluteFilepath);

          PointerHeapArray__pushBack(allComptimeGeneratedFiles, strdup(absoluteFilepath));

          // {
          //   // register the relationship between comptime generator/generated files

          //   // register the relationship between "comptime calling files" and the "comptime generated file"

          //   // get or create HashSet
          //   HashSet* tmpSet = HashMap__get(self->allExtraIncludesPerFiles, currFile->filepath);
          //   if (!tmpSet) {
          //     tmpSet = HashSet__preAllocate(32);
          //     HashMap__set(self->allExtraIncludesPerFiles, currFile->filepath, tmpSet);
          //   }
          //   // add the relationship
          //   HashSet__set(tmpSet, absoluteFilepath);

          //   printf(" => GENERATED_REGISTER %s -> %s\n", currFile->filepath, absoluteFilepath);
          // }

          snprintf(buffer, 1024, "\n    redirect_stdout(\"%s\");\n", absoluteFilepath);
          // snprintf(buffer, 1024, "\n    redirect_stdout(\"%s\");\n", relativeFilepath);
          StreamWriter__write(streamWriter, buffer, strlen(buffer));

          // free(relativeFilepath);
          free(absoluteFilepath);
          free(comptimeFuncDefFilename);
          free(comptimeFuncDefGeneratedFolderPath);
          free(comptimeFuncDefFolderPath);
        }

        // write the function call

        {
          snprintf(buffer, 1024, "    %s(", currComptime->varName);
          StreamWriter__write(streamWriter, buffer, strlen(buffer));
        }

        for (unsigned int jj = 0; jj < currComptime->argsList.len; ++jj)
        {
          const StringData* currArgStr = &currComptime->argsList.data[jj];

          if (jj > 0)
          {
            snprintf(buffer, 1024, ", ");
            StreamWriter__write(streamWriter, buffer, strlen(buffer));
          }

          {
            snprintf(buffer, 1024, "\"%s\"", currArgStr->data);
            StreamWriter__write(streamWriter, buffer, strlen(buffer));
          }
        }

        {
          snprintf(buffer, 1024, ");\n");
          StreamWriter__write(streamWriter, buffer, strlen(buffer));
        }
      }

      free(items);
    }


    {
      const char* k_func = "\n"
        "\n"
        "    fflush(stdout);\n"
        "    // back to the terminal\n"
        "    dup2(saved, STDOUT_FILENO);\n"
        "    close(saved);\n"
        "    printf(\"COMPTIME DONE!\\n\");\n"
        "\n"
        "    return 0;\n"
        "}\n"
        "\n";

      StreamWriter__write(streamWriter, k_func, strlen(k_func));
    }

    StreamWriter__free(&streamWriter);
  }

  {
    SourceAnalyzer* tmpAnalyzer = SourceAnalyzer__create(0);
    SourceAnalyzer__scanFromMainFile(tmpAnalyzer, comptimeMainFilepath, inIncludePath);

    // debug
    {
      const PointerHeapArray * sortedAnalyzedFiles = SourceAnalyzer__getSortedAnalyzedFiles(tmpAnalyzer);
      printf(" -> totalTmpAnalyzed:%d\n", sortedAnalyzedFiles->len);
      for (unsigned int ii = 0; ii < sortedAnalyzedFiles->len; ++ii)
      {
        const AnalyzedFile* analyzedFile = sortedAnalyzedFiles->data[ii];
        printf("   -> tmpAnalyzed[%d]: %s\n", ii, analyzedFile->filepath);
      }
    }

    // unsigned int totalAnalyzed;
    // char** allAnalyzed = SourceAnalyzer__getAllAnalyzed(tmpAnalyzer, &totalAnalyzed);
    const PointerHeapArray * sortedAnalyzedFiles2 = SourceAnalyzer__getSortedAnalyzedFiles(tmpAnalyzer);

    for (unsigned int ii = 0; ii < sortedAnalyzedFiles2->len; ++ii)
    {
      // const AnalyzedFile* analyzedFile = SourceAnalyzer__getAnalyzed(tmpAnalyzer, allAnalyzed[ii]);
      const AnalyzedFile* analyzedFile = sortedAnalyzedFiles2->data[ii];

      _Transpiler__processFile(self, analyzedFile);
    }

    {
      char* outSrcFolder = Path__join(2, self->outputDir, "src");

      PointerHeapArray* sourcesFilepaths = PointerHeapArray__preAllocate(32);
      // PointerHeapArray__pushBack(sourcesFilepaths, comptimeMainFilepath);

      unsigned int totalKeys = 0;
      char** allKeys = HashSet__get_allKeys(self->allGeneratedFiles, &totalKeys);

      for (unsigned int ii = 0; ii < totalKeys; ++ii) {

        // const int baseDirLen = strlen(outSrcFolder);
        // const char* relInputFilepath = allKeys[ii] + baseDirLen;
        // char* outFilepath = Path__join(2, outSrcFolder, relInputFilepath);

        // PointerHeapArray__pushBack(sourcesFilepaths, allAnalyzed[ii]);
        // PointerHeapArray__pushBack(sourcesFilepaths, outFilepath);
        PointerHeapArray__pushBack(sourcesFilepaths, strdup(allKeys[ii]));
      }

      free(allKeys);

      BuildCmakeConfigOpts buildOpts;
      buildOpts.inBaseDir = outSrcFolder;
      buildOpts.inSourcesFilepaths = sourcesFilepaths;
      buildOpts.inOutputDir = self->outputDir;
      buildOpts.inIncludePath = NULL;
      buildOpts.inLibraryPath = NULL;
      buildOpts.doBuild = 1;
      buildOpts.strictMode = 1;

      const int result = BuildCmakeConfig__generateCmakeFile(&buildOpts);
      if (result != 0) {
        // TODO
        panic("failed to build the comptime code generation binary");
      }
      else {
        char *const argv[] = { NULL };
        const int result = executeCommand("./bin/exec", argv, self->outputDir);
        if (result != 0) {
          // TODO
          panic("failed to run the comptime code generation binary");
        }
      }

      for (unsigned int ii = 0; ii < sourcesFilepaths->len; ++ii)
      {
        char* tmpStr = sourcesFilepaths->data[ii];
        free(tmpStr);
      }

      PointerHeapArray__free(&sourcesFilepaths);

      free(outSrcFolder);
    }

    // free(allAnalyzed);

    SourceAnalyzer__free(&tmpAnalyzer);
  }

  // add the generated file to the transpiler
  for (unsigned int ii = 0; ii < allComptimeGeneratedFiles->len; ++ii)
  {
    char* filepathStr = allComptimeGeneratedFiles->data[ii];

    // if (SourceAnalyzer__scanFile(self->analyzer, filepathStr, NULL) < 0)
    if (SourceAnalyzer__scanFile(self->analyzer, filepathStr, inIncludePath) < 0)
    {
      fprintf(stderr, "\nFATAL ERROR:\n -> failed to scan the comptime generated source code\n ---> %s\n", filepathStr);
      panic("failed to scan the comptime generated source code");
    }
  }

  for (unsigned int ii = 0; ii < allComptimeGeneratedFiles->len; ++ii)
  {
    char* filepathStr = allComptimeGeneratedFiles->data[ii];

    const AnalyzedFile* analyzedFile = SourceAnalyzer__getAnalyzed(self->analyzer, filepathStr);

    _Transpiler__processFile(self, analyzedFile);
  }

  for (unsigned int ii = 0; ii < allComptimeGeneratedFiles->len; ++ii)
  {
    char* filepathStr = allComptimeGeneratedFiles->data[ii];
    free(filepathStr);
  }
  PointerHeapArray__free(&allComptimeGeneratedFiles);


  free(comptimeMainFilepath);

  {
    // for (unsigned int ii = 0; ii < allFilesToInclude->len; ++ii)
    // {
    //   char* currFilepath = allFilesToInclude->data[ii];
    //   free(currFilepath);
    // }

    PointerHeapArray__free(&allFilesToInclude);
  }

  HashSet__free(&allUniqueComptimeNameSet);
  HashMap__free(&allUniqueComptimeCallMap);

  free(generatedFolderPath);

  printf("COMPTIME DONE\n");

  printf("########\n");
  printf("######\n");
  printf("####\n");
  printf("##\n");

  // free(allAnalyzed);
  return 0;
}



//MARK: some stuff
typedef enum EditType {
  ET_COMMENT_RANGE,
  ET_CONVERT_RANGE,
  ET_CONVERT_SUPER_STRING_LITERAL,
} EditType;

typedef struct CommentRangeEdit {
  unsigned int startIndex;
  unsigned int endIndex;
} CommentRangeEdit;

typedef struct ConvertRangeEdit {
  unsigned int startIndex;
  unsigned int endIndex;
  char* replacement;
} ConvertRangeEdit;

typedef struct CommentRangeEdit SuperStringLiteralEdit;

typedef union EditUnion {
  CommentRangeEdit commentRange;
  ConvertRangeEdit convertRange;
  SuperStringLiteralEdit superStringLiteral;
} EditUnion;

typedef struct AnyEdit {
  EditType editType;
  EditUnion editValue;
} AnyEdit;


const char* k_queryExportedFuncDefStr = "\n"
  "\n"
  "; includes\n"
  "\n"
  "(preproc_include path: (_) @import.source) @import\n"
  "\n"
  "\n"
  "; functions definitions\n"
  "\n"
  "(function_definition (\"export\")? @exported (\"comptime\")? @comptime (\"test\")? @test type: (_) @return.type\n"
  "  declarator: [\n"
  "    (\n"
  "      function_declarator declarator: (identifier) @name)\n"
  "    (pointer_declarator (\"*\") @pointer.level declarator: (\n"
  "      function_declarator declarator: (identifier) @name))\n"
  "    (pointer_declarator (\"*\") @pointer.level declarator: (pointer_declarator (\"*\") @pointer.level2 declarator: (\n"
  "      function_declarator declarator: (identifier) @name)))\n"
  "    (pointer_declarator (\"*\") @pointer.level declarator: (pointer_declarator (\"*\") @pointer.level2 (pointer_declarator (\"*\") @pointer.level3 declarator: (\n"
  "      function_declarator declarator: (identifier) @name))))\n"
  "  ]\n"
  "  (_) @body) @definition.function\n"
  "\n"
  "\n"
  "; Structs, Unions, Enums, Typedefs\n"
  "\n"
  "(struct_specifier (\"export\")? @exported name: (type_identifier) @name templated_type: (type_identifier)? @templated.type (field_declaration_list) @field_declaration_list) @definition.struct\n"
  "(enum_specifier (\"export\")? @exported name: (type_identifier) @name) @definition.enum\n"
  "(union_specifier (\"export\")? @exported name: (type_identifier) @name) @definition.union\n"
  "(type_definition (\"export\")? @exported declarator: (type_identifier) @name) @definition.typedef\n"
  "\n"
  "\n"
  "; Super string literals\n"
  "\n"
  "(super_string_literal) @super.string.literal\n"
  "\n"
  "; Static method call\n"
  "\n"
  "(static_call_expression (statement_identifier) @namespace (call_expression function: ((identifier) @call.name (_) @call.args))) @static.call\n"
  "(static_call_expression (comptime_call_expression) @namespace (call_expression function: ((identifier) @call.name (_) @call.args))) @static.call\n"
  "\n"
  "; Non-Static method call\n"
  "\n"
  "(call_expression function: (field_expression argument: (identifier) @call.caller field: (field_identifier) @call.callee) ((_) @call.args) ) @method.call\n"
  "\n"
  "\n";

//MARK: _processFile
static int _Transpiler__processFile(Transpiler* self, const AnalyzedFile* inAnalyzedFile)
{
  const char* filepath = AnalyzedFile__getFilepath(inAnalyzedFile);

  if (HashSet__contains(self->alreadyProcessedFiles, filepath))
  {
    return 0;
  }
  HashSet__set(self->alreadyProcessedFiles, filepath);

  // printf(" -> processing file:\n   -> \"%s\"\n", filepath);
  printf(" -> processing file: \"%s\"\n", filepath);

  StopWatch stopWatch = StopWatch__create();
  StopWatch__start(&stopWatch);

  // self->baseDir
  // self->entryFilepath
  // self->outputDir

  char* outSrcFolder = Path__join(2, self->outputDir, "src");
  char* inputFilepathExt = Path__extname(filepath);

  const int baseDirLen = strlen(self->baseDir);
  const char* relInputFilepath = filepath + baseDirLen;
  char* outFilepath = Path__join(2, outSrcFolder, relInputFilepath);
  char* outHeaderFilepath = NULL;

  if (strcmp(inputFilepathExt, ".lc") == 0)
  {
    const int outFilepathLen = strlen(outFilepath);

    StringBuffer* strbuffer = StringBuffer__create();
    StringBuffer__appendData(strbuffer, outFilepathLen - 3, outFilepath);
    StringBuffer__appendCString(strbuffer, ".c");

    free(outFilepath);
    outFilepath = strndup(strbuffer->data, strbuffer->len);

    strbuffer->len -= 2;
    StringBuffer__appendCString(strbuffer, ".h");
    outHeaderFilepath = strndup(strbuffer->data, strbuffer->len);

    StringBuffer__free(&strbuffer);
  }

  const SourceParsedFile* parsedFile = AnalyzedFile__getParsedFile(inAnalyzedFile);
  const char* fileContent = SourceParsedFile__getFileContent(parsedFile);

  // source file data
  PointerHeapArray* allEdits = PointerHeapArray__preAllocate(32);
  // header file data
  PointerHeapArray* allTmpImports = PointerHeapArray__preAllocate(32);
  // header file data
  PointerHeapArray* allStructs = PointerHeapArray__preAllocate(32);
  // header file data
  PointerHeapArray* allSignatures = PointerHeapArray__preAllocate(32);

  SourceParser* parser = SourceAnalyzer__getParser(self->analyzer);

  // TSQuery* queryX = SourceParser__getQueryX(parser);
  TSQuery* queryX = SourceParser__parseQuery(parser, "queryX", k_queryExportedFuncDefStr);

  // get the exported function definitions
  // QueryMatchData* newMatchData = SourceParsedFile__query(parsedFile, k_queryExportedFuncDefStr, strlen(k_queryExportedFuncDefStr));
  QueryMatchData* newMatchData = SourceParsedFile__query(parsedFile, queryX);

  ///MARK: collect edits
  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {

    if (
      HashMap__contains(cursor->captureMap, "import") &&
      HashMap__contains(cursor->captureMap, "import.source")
    ) {
      const NodeData* sourceNode = HashMap__get(cursor->captureMap, "import.source");

      const char* srcStr = fileContent + sourceNode->startPos.index;
      const unsigned int contentSize = sourceNode->endPos.index - sourceNode->startPos.index;

      if (srcStr[0] == '\"' && srcStr[contentSize - 1] == '\"') {
        // ex -> #include "some-filepath.h"

        char* includedPathStr = strndup(srcStr + 1, contentSize - 2);
        char* includedPathExtStr = Path__extname(includedPathStr);

        // printf(" IMPORT_SOURCE ---> {%s} (%s)\n", includedPathStr, includedPathExtStr);

        if (
          strcmp(includedPathExtStr, ".c") == 0 ||
          strcmp(includedPathExtStr, ".lc") == 0
        ) {

          StringBuffer* strbuffer = StringBuffer__create();
          StringBuffer__appendData(strbuffer, contentSize - 2 - strlen(includedPathExtStr), includedPathStr);
          StringBuffer__appendCString(strbuffer, ".h");

          char buffer[1024];
          memset(buffer, 0, 1024);
          // snprintf(buffer, 1024, "\"%s\"\n", strbuffer->data);
          snprintf(buffer, 1024, "\"%s\"", strbuffer->data);

          {
            // add the edit
            AnyEdit* newEdit = calloc(1, sizeof(AnyEdit));
            newEdit->editType = ET_CONVERT_RANGE;
            newEdit->editValue.convertRange.startIndex = sourceNode->startPos.index;
            newEdit->editValue.convertRange.endIndex = sourceNode->endPos.index;
            newEdit->editValue.convertRange.replacement = strdup(buffer);
            PointerHeapArray__pushBack(allEdits, newEdit);
          }

          // printf("  -> IMPORT EDIT ---> replacement=[%s]\n", buffer);

          StringBuffer__free(&strbuffer);

          // ex -> #include "..."
          PointerHeapArray__pushBack(allTmpImports, strdup(buffer));
        }
        else
        {
          // ex -> #include "..."
          PointerHeapArray__pushBack(allTmpImports, strndup(srcStr, contentSize));
        }

        free(includedPathExtStr);
        free(includedPathStr);
      }
      else
      {
        // ex -> #include <...>
        PointerHeapArray__pushBack(allTmpImports, strndup(srcStr, contentSize));
      }
    }
    else if (
      HashMap__contains(cursor->captureMap, "definition.function") &&
      // HashMap__contains(cursor->captureMap, "name") &&
      HashMap__contains(cursor->captureMap, "return.type") &&
      HashMap__contains(cursor->captureMap, "body") //&&
      // HashMap__contains(cursor->captureMap, "exported")
    ) {

      {
        // handle exported function
        const NodeData* exportedNode = HashMap__get(cursor->captureMap, "exported");
        const NodeData* comptimeNode = HashMap__get(cursor->captureMap, "comptime");
        if (exportedNode || comptimeNode) {

          {
            unsigned int startIndex = 0;
            unsigned int endIndex = 0;

            if (exportedNode) {
              startIndex = exportedNode->startPos.index;
            } else {
              startIndex = comptimeNode->startPos.index;
            }

            if (comptimeNode) {
              endIndex = comptimeNode->endPos.index;
            } else {
              endIndex = exportedNode->endPos.index;
            }

            // add the edit
            AnyEdit* newEdit = calloc(1, sizeof(AnyEdit));
            newEdit->editType = ET_COMMENT_RANGE;
            newEdit->editValue.commentRange.startIndex = startIndex;
            newEdit->editValue.commentRange.endIndex = endIndex;
            PointerHeapArray__pushBack(allEdits, newEdit);
          }

          {
            // save the signature
            const NodeData* returnTypeNode = HashMap__get(cursor->captureMap, "return.type");
            const NodeData* bodyNode = HashMap__get(cursor->captureMap, "body");

            // printf(" ---> returnTypeNode: \"%s\"\n", returnTypeNode->captureName.data);
            // printf(" ---> bodyNode:       \"%s\"\n", bodyNode->captureName.data);

            const char* funcSignatureData = fileContent + returnTypeNode->startPos.index;
            const int funcSignatureLen = (bodyNode->startPos.index - returnTypeNode->startPos.index);

            char* newFuncSignature = strndup(funcSignatureData, funcSignatureLen);

            // // printf(" -----> SIGNATURE[%d]: \"%*.s\"\n", signatureLen, signatureLen, signature);
            // printf(" -----> SIGNATURE: \"%s\"\n", newFuncSignature);

            PointerHeapArray__pushBack(allSignatures, newFuncSignature);
          }

        }
      }

    }
    else if (
      HashMap__contains(cursor->captureMap, "definition.struct") ||
      HashMap__contains(cursor->captureMap, "definition.enum") ||
      HashMap__contains(cursor->captureMap, "definition.union") ||
      HashMap__contains(cursor->captureMap, "definition.typedef")
    ) {

      {
        // handle exported struct
        const NodeData* exportedNode = HashMap__get(cursor->captureMap, "exported");
        if (exportedNode) {

          int isTypeDef = 0;

          NodeData* mainNode = HashMap__get(cursor->captureMap, "definition.struct");
          if (!mainNode) { mainNode = HashMap__get(cursor->captureMap, "definition.enum"); }
          if (!mainNode) { mainNode = HashMap__get(cursor->captureMap, "definition.union"); }
          if (!mainNode) { mainNode = HashMap__get(cursor->captureMap, "definition.typedef"); isTypeDef = 1; }

          {
            // add the edit
            AnyEdit* newEdit = calloc(1, sizeof(AnyEdit));
            newEdit->editType = ET_COMMENT_RANGE;
            newEdit->editValue.commentRange.startIndex = mainNode->startPos.index;
            newEdit->editValue.commentRange.endIndex = mainNode->endPos.index;
            PointerHeapArray__pushBack(allEdits, newEdit);
          }

          {
            // save the definition
            // printf(" ---> struct.mainNode: \"%s\"\n", mainNode->captureName.data);

            const char* structData = fileContent + exportedNode->endPos.index;
            const int structLen = (mainNode->endPos.index - exportedNode->endPos.index);
            char* newStruct = strndup(structData, structLen);

            if (isTypeDef == 0)
            {
              NodeData* nameNode = HashMap__get(cursor->captureMap, "name");

              const char* nameData = fileContent + nameNode->startPos.index;
              const int nameLen = (nameNode->endPos.index - nameNode->startPos.index);
              char* nameStr = strndup(nameData, nameLen);

              unsigned int tmpLen = structLen + nameLen + 10;
              char* tmpBuf = calloc(tmpLen, sizeof(char));

              snprintf(tmpBuf, tmpLen, "typedef%s %s", newStruct, nameStr);

              free(newStruct);
              newStruct = tmpBuf;

              free(nameStr);
            }

            // printf(" -----> SIGNATURE[%d]: \"%*.s\"\n", signatureLen, signatureLen, signature);
            // printf(" -----> SIGNATURE: \"%s\"\n", newStruct);

            PointerHeapArray__pushBack(allStructs, newStruct);
          }

        }
      }
    }
    else if (
      HashMap__contains(cursor->captureMap, "super.string.literal")
    ) {
      const NodeData* mainNode = HashMap__get(cursor->captureMap, "super.string.literal");

      {
        // add the edit
        AnyEdit* newEdit = calloc(1, sizeof(AnyEdit));
        newEdit->editType = ET_CONVERT_SUPER_STRING_LITERAL;
        newEdit->editValue.superStringLiteral.startIndex = mainNode->startPos.index;
        newEdit->editValue.superStringLiteral.endIndex = mainNode->endPos.index;
        PointerHeapArray__pushBack(allEdits, newEdit);
      }
    }
    else if (
      HashMap__contains(cursor->captureMap, "static.call")
    ) {

      const NodeData* namespaceNode = HashMap__get(cursor->captureMap, "namespace");
      const NodeData* callNameNode = HashMap__get(cursor->captureMap, "call.name");

      {
        // add the edit
        AnyEdit* newEdit = calloc(1, sizeof(AnyEdit));
        newEdit->editType = ET_CONVERT_RANGE;
        newEdit->editValue.convertRange.startIndex = namespaceNode->endPos.index;
        newEdit->editValue.convertRange.endIndex = callNameNode->startPos.index;
        newEdit->editValue.convertRange.replacement = strdup("__");
        PointerHeapArray__pushBack(allEdits, newEdit);
      }

    }
    else if(
      HashMap__contains(cursor->captureMap, "method.call") &&
      HashMap__contains(cursor->captureMap, "call.caller") &&
      HashMap__contains(cursor->captureMap, "call.callee") &&
      // HashMap__contains(cursor->captureMap, "call.op") &&
      HashMap__contains(cursor->captureMap, "call.args")
    ) {

      printf(" -> METHOD CALL\n");

      // const NodeData* callNode = HashMap__get(cursor->captureMap, "method.call");
      const NodeData* callerNode = HashMap__get(cursor->captureMap, "call.caller");
      const NodeData* calleeNode = HashMap__get(cursor->captureMap, "call.callee");
      // const NodeData* opNode = HashMap__get(cursor->captureMap, "call.op");
      const NodeData* argsNode = HashMap__get(cursor->captureMap, "call.args");

      // const NodeData* allNodes[4] = {
      //   callNode,
      //   callerNode,
      //   calleeNode,
      //   // opNode,
      //   argsNode
      // };

      // for (unsigned int ii = 0; ii < 4; ++ii)
      // {
      //   if (!allNodes[ii]) {
      //     continue;
      //   }

      //   {
      //     const char* pStrData = fileContent + allNodes[ii]->startPos.index;
      //     const int strLen = (allNodes[ii]->endPos.index - allNodes[ii]->startPos.index);

      //     char* newStr = strndup(pStrData, strLen);

      //     printf(" -----> { %s }\n", newStr);

      //     free(newStr);
      //   }

      // }

      const char* pcallerStrData = fileContent + callerNode->startPos.index;
      const int callerStrLen = (callerNode->endPos.index - callerNode->startPos.index);

      char* callerStr = strndup(pcallerStrData, callerStrLen);

      printf(" ---> [ callerStr=%s ]\n", callerStr);

      const VarDef* varDef = SourceIndexer__findTypename(inAnalyzedFile->indexer, callerStr, callerNode->startPos);
      if (varDef)
      {
        printf(" ---> [ callerTypeName=%s ]\n", varDef->typeName);

        //

        {
          // change the "method call" to a "function call"

          unsigned int tmpLen = strlen(varDef->typeName) + 3;
          char* tmpBuf = calloc(tmpLen, sizeof(char));

          snprintf(tmpBuf, tmpLen, "%s__", varDef->typeName);

          {
            // add the edit
            AnyEdit* newEdit = calloc(1, sizeof(AnyEdit));
            newEdit->editType = ET_CONVERT_RANGE;
            newEdit->editValue.convertRange.startIndex = callerNode->startPos.index;
            newEdit->editValue.convertRange.endIndex = calleeNode->startPos.index;
            newEdit->editValue.convertRange.replacement = tmpBuf;
            PointerHeapArray__pushBack(allEdits, newEdit);
          }

          // free(tmpBuf);
        }

        {
          // handle the required first argument

          // varDef->pointerLevel

          // TODO: identify the function being called and determine the pointerLevel
          const int desiredPointerLevel = 1;
          if (varDef->pointerLevel != desiredPointerLevel)
          {
            // TODO
          }
          else
          {
            // TODO
          }

          unsigned int tmpLen = strlen(callerStr) + 3;
          char* tmpBuf = calloc(tmpLen, sizeof(char));

          unsigned int argStrLen = argsNode->endPos.index - argsNode->startPos.index;
          if (argStrLen == 2) {
            // no args in the call
            snprintf(tmpBuf, tmpLen, "%s", callerStr);
          } else {
            // the call has at least one arg
            snprintf(tmpBuf, tmpLen, "%s, ", callerStr);
          }

          {
            // add the edit
            AnyEdit* newEdit = calloc(1, sizeof(AnyEdit));
            newEdit->editType = ET_CONVERT_RANGE;
            newEdit->editValue.convertRange.startIndex = argsNode->startPos.index + 1;
            newEdit->editValue.convertRange.endIndex = argsNode->startPos.index + 1;
            newEdit->editValue.convertRange.replacement = tmpBuf;
            PointerHeapArray__pushBack(allEdits, newEdit);
          }

          // free(tmpBuf);
        }


      }
      else
      {
        printf(" ---> [ callerTypeName=NULL ]\n");
      }



      free(callerStr);
    }

  }

  // determine if a header file is possible and needed
  const int needHeaderFile = (
    outHeaderFilepath && (
      allStructs->len > 0 ||
      allSignatures->len > 0
    )
  ) ? 1 : 0;

  // printf("     -> needHeaderFile: %d\n", needHeaderFile);

  //
  //
  //

  // apply the edits
  // -> in-file-content: fileContent
  // -> out-file-path: outFilepath

  {
    char* outFolderPath = Path__dirname(outFilepath);
    if (ensureFolder(outFolderPath) != 0)
    {
      // TODO: need a goto to a failure_return
      return -1;
    }
    free(outFolderPath);
  }

  ///MARK: make C
  StreamWriter* streamWriter = StreamWriter__create(outFilepath);
  if (!streamWriter) {
    fprintf(stderr, " -> streamWriter failure, PATH=%s\n", outFilepath);
    return -1;
  }

  if (needHeaderFile != 0)
  {
    {
      // prepend the include of the header to its C file
      // -> done since any exported struct/union/enum will be moved to the header

      char* baseHeaderFile = Path__basename(outHeaderFilepath);

      const int bufSize = strlen(outHeaderFilepath) + 32;
      char* tmpBuf = calloc(bufSize, sizeof(char));

      snprintf(tmpBuf, bufSize, "\n#include \"./%s\"\n\n", baseHeaderFile);
      StreamWriter__write(streamWriter, tmpBuf, strlen(tmpBuf));

      free(tmpBuf);
      free(baseHeaderFile);
    }
  }

    {
      // add the extra "#include" (<- comptime generated files)
      HashSet* tmpSet = HashMap__get(self->allExtraIncludesPerFiles, filepath);
      if (tmpSet && HashSet__get_totalItems(tmpSet) > 0)
      {
        char* outFolderPath = Path__dirname(filepath);

        printf(" GOT GENERATED STUFF: %s\n", filepath);

        unsigned int totalKeys = 0;
        char** allkeys = HashSet__get_allKeys(tmpSet, &totalKeys);

        for (unsigned int ii = 0; ii < totalKeys; ++ii)
        {
          const char* tmpFilepath = allkeys[ii];
          char* relPath = Path__relative(outFolderPath, tmpFilepath);

          printf("  -> IS GENERATED\n");
          printf("    -> caller: %s\n", filepath);
          printf("    -> callee: %s\n", tmpFilepath);
          printf("    -> relative: %s\n", relPath);

          const int bufSize = strlen(relPath) + 32;
          char* tmpBuf = calloc(bufSize, sizeof(char));

          snprintf(tmpBuf, bufSize, "\n#include \"%s\"\n\n", relPath);
          StreamWriter__write(streamWriter, tmpBuf, strlen(tmpBuf));

          free(tmpBuf);
          free(relPath);
        }

        free(allkeys);
        free(outFolderPath);
      }
    }
  // }

  const int fileLength = strlen(fileContent);

  // printf(" ====> fileLength=%d\n", fileLength);

  int currReadIndex = 0;
  for (unsigned int ii = 0; ii < allEdits->len; ++ii) {
    AnyEdit* currEdit = allEdits->data[ii];

    switch (currEdit->editType)
    {
      case ET_COMMENT_RANGE:
      {
        CommentRangeEdit* commentRange = &currEdit->editValue.commentRange;
        // printf("  -> ET_COMMENT_RANGE\n");

        {
          // advance to the start of the next edit
          const char* start = fileContent + currReadIndex;
          const int length = commentRange->startIndex - currReadIndex;
          char* tmpStr = strndup(start, length);
          StreamWriter__write(streamWriter, tmpStr, length);
          free(tmpStr);
          currReadIndex += length;
        }

        {
          // inject comment-start
          char* tmpStr = strndup("/*", 2);
          StreamWriter__write(streamWriter, tmpStr, 2);
          free(tmpStr);
        }

        {
          // advance to the end of the current edit
          const char* start = fileContent + commentRange->startIndex;
          const int length = commentRange->endIndex - commentRange->startIndex;
          char* tmpStr = strndup(start, length);
          StreamWriter__write(streamWriter, tmpStr, length);
          free(tmpStr);
          currReadIndex += length;
        }

        {
          // inject comment-end
          char* tmpStr = strndup("*/", 2);
          StreamWriter__write(streamWriter, tmpStr, 2);
          free(tmpStr);
        }

        break;
      }
      case ET_CONVERT_RANGE:
      {
        ConvertRangeEdit* convertRange = &currEdit->editValue.convertRange;
        // printf("  -> ET_CONVERT_RANGE (%s)\n", convertRange->replacement);

        {
          // advance to the start of the next edit
          const char* start = fileContent + currReadIndex;
          const int length = convertRange->startIndex - currReadIndex;
          char* tmpStr = strndup(start, length);
          StreamWriter__write(streamWriter, tmpStr, length);
          free(tmpStr);
          currReadIndex += length;
        }

        {
          // advance to the end of the current edit but write the replacement string instead
          StreamWriter__write(streamWriter, convertRange->replacement, strlen(convertRange->replacement));
          const int length = convertRange->endIndex - convertRange->startIndex;
          currReadIndex += length;
        }

        break;
      }
      case ET_CONVERT_SUPER_STRING_LITERAL:
      {
        SuperStringLiteralEdit* superStringLiteral = &currEdit->editValue.superStringLiteral;
        // printf("  -> ET_CONVERT_SUPER_STRING_LITERAL\n");

        // remove the prefixed and suffixed ```
        // handle each lines

        {
          // advance to the start of the next edit
          const char* start = fileContent + currReadIndex;
          const int length = superStringLiteral->startIndex - currReadIndex;
          char* tmpStr = strndup(start, length);
          StreamWriter__write(streamWriter, tmpStr, length);
          free(tmpStr);
          currReadIndex += length;
        }

        // {
        //   // ignore the prefixed ```
        //   currReadIndex += 3;
        // }

        {

          // ignore the prefixed and suffixed ```
          const char* start = fileContent + superStringLiteral->startIndex + 3;
          const int length = (superStringLiteral->endIndex - superStringLiteral->startIndex) - 6;
          char* fullContent = strndup(start, length);

          // extract the all lines

          char *cursor = fullContent;
          for (;;) {
            if ((int)(cursor - fullContent) >= length) {
              break;
            }
            // char *matchStr = strchr(cursor, '\n');

            char *matchStr = NULL;
            int doubleQuoteCounter = 0;
            for (unsigned int kk = 0; cursor[kk]; ++kk)
            {
              if (cursor[kk] == '\n' && doubleQuoteCounter == 0)
              {
                matchStr = cursor + kk;
                break;
              }
              else if (cursor[kk] == '\"') {
                doubleQuoteCounter = doubleQuoteCounter ? 0 : 1;
              }
            }

            if (matchStr == NULL)
            {
              break;
            }

            const unsigned int matchLen = (unsigned int)(matchStr - cursor);

            if (matchLen == 0) {
              // empty line
              StreamWriter__write(streamWriter, "\"\\n\"\n", 5);
              cursor = matchStr + 1;
            }
            else {
              char* currLineStr = strndup(cursor, matchLen);

              // printf(" ---> currLineStr=%s\n", currLineStr);

              // unsigned int cleanLineLen = matchLen * 2;
              // char* cleanLineStrA = calloc(cleanLineLen, sizeof(char));
              // char* cleanLineStrB = calloc(cleanLineLen, sizeof(char));

              // // -> \" -> \\\"
              // // -> \n -> '\\n'
              // String__replaceAll(currLineStr, "\"", "\\\"", cleanLineStrA, cleanLineLen);
              // String__replaceAll(cleanLineStrA, "\n", "\\n", cleanLineStrB, cleanLineLen);
              StringData cleanLineStrA = String__replaceAll2(currLineStr, "\"", "\\\"");
              StringData cleanLineStrB = String__replaceAll2(cleanLineStrA.data, "\n", "\\n");

              // printf(" ---> cleanLineStrA.data=%s\n", cleanLineStrA.data);
              // printf("   -> cleanLineStrB.data=%s\n", cleanLineStrB.data);

              StreamWriter__write(streamWriter, "\"", 1);
              // StreamWriter__write(streamWriter, cleanLineStrB.data, strlen(cleanLineStrB.data));
              StreamWriter__write(streamWriter, cleanLineStrB.data, cleanLineStrB.len);
              StreamWriter__write(streamWriter, "\\n\"\n", 4);

              // free(cleanLineStrB);
              // free(cleanLineStrA);
              free(cleanLineStrB.data);
              free(cleanLineStrA.data);
              free(currLineStr);

              cursor = matchStr + 1;
            }

          }

          StreamWriter__write(streamWriter, "\"\"", 2);

          free(fullContent);
          currReadIndex += length + 3;
        }


        {
          // ignore the suffixed ```
          currReadIndex += 3;
        }

        break;
      }
    }
  }

  // printf("   ==> left=%d/%d\n", currReadIndex, fileLength);
  if (currReadIndex < fileLength) {
    const char* start = fileContent + currReadIndex;
    const int length = fileLength - currReadIndex;
    char* tmpStr = strndup(start, length);
    StreamWriter__write(streamWriter, tmpStr, length);
    free(tmpStr);
    currReadIndex += length;
  }

  // printf("    => left=%d/%d\n", currReadIndex, fileLength);

  StreamWriter__free(&streamWriter);



  HashSet__set(self->allGeneratedFiles, outFilepath);

  //
  //
  //

  ///MARK: make H
  if (needHeaderFile != 0) {
    // generate header file

    StreamWriter* streamWriter = StreamWriter__create(outHeaderFilepath);
    if (!streamWriter) {
      fprintf(stderr, " -> streamWriter failure, PATH=%s\n", outHeaderFilepath);
      return -1;
    }

    {
      const char* k_str = "\n"
        "#pragma once\n"
        "\n";

      StreamWriter__write(streamWriter, k_str, strlen(k_str));
    }


    {
      // add the extra "#include" (<- comptime generated files)
      HashSet* tmpSet = HashMap__get(self->allExtraIncludesPerFiles, filepath);
      if (tmpSet && HashSet__get_totalItems(tmpSet) > 0)
      {
        char* outFolderPath = Path__dirname(filepath);

        printf(" GOT GENERATED STUFF: %s\n", filepath);

        unsigned int totalKeys = 0;
        char** allkeys = HashSet__get_allKeys(tmpSet, &totalKeys);

        for (unsigned int ii = 0; ii < totalKeys; ++ii)
        {
          const char* tmpFilepath = allkeys[ii];
          char* relPath = Path__relative(outFolderPath, tmpFilepath);

          printf("  -> IS GENERATED\n");
          printf("    -> caller: %s\n", filepath);
          printf("    -> callee: %s\n", tmpFilepath);
          printf("    -> relative: %s\n", relPath);

          const int bufSize = strlen(relPath) + 32;
          char* tmpBuf = calloc(bufSize, sizeof(char));

          snprintf(tmpBuf, bufSize, "\n#include \"%s\"\n\n", relPath);
          StreamWriter__write(streamWriter, tmpBuf, strlen(tmpBuf));

          free(tmpBuf);
          free(relPath);
        }

        free(allkeys);
        free(outFolderPath);
      }
    }

    {
      const HashSet* typesToResolve = SourceIndexer__getComptimeTypesToResolve(inAnalyzedFile->indexer);

      if (HashSet__get_totalItems(typesToResolve) > 0)
      {
        unsigned int totalKeys = 0;
        char** allKeys = HashSet__get_allKeys(typesToResolve, &totalKeys);

        char* outFolderPath = Path__dirname(filepath);

        for (unsigned int ii = 0; ii < totalKeys; ++ii)
        {
          const char* currTypename = allKeys[ii];
          const AnalyzedFile* targetAnalyzedFile = SourceAnalyzer__getAnalyzedFromDefinitionName(self->analyzer, currTypename);

          if (!targetAnalyzedFile)
          {
            fprintf(stderr, "type to resolve not found: %s\n", currTypename);
            panic("type to resolve not found");
          }

          const char* tmpFilepath = targetAnalyzedFile->filepath;
          char* relPath = Path__relative(outFolderPath, tmpFilepath);

          char* extname = Path__extname(relPath);
          if (strcmp(extname, ".lc") == 0)
          {
            const int bufSize = strlen(relPath) + 5;
            char* headerFilepath = calloc(bufSize, sizeof(char));

            char* tmpStr = strndup(relPath, strlen(relPath) - 3);
            snprintf(headerFilepath, bufSize, "%s.h", tmpStr);
            free(tmpStr);

            // printf(" -> headerFilepath=%s\n", headerFilepath);

            free(relPath);
            relPath = headerFilepath;
          }

          free(extname);

          const int bufSize = strlen(outHeaderFilepath) + 32;
          char* tmpBuf = calloc(bufSize, sizeof(char));
          snprintf(tmpBuf, bufSize, "\n#include \"%s\"\n\n", relPath);
          StreamWriter__write(streamWriter, tmpBuf, strlen(tmpBuf));
          free(tmpBuf);

          free(relPath);
        }

        free(outFolderPath);
        free(allKeys);
      }
    }

    {
      for (unsigned int ii = 0; ii < allTmpImports->len; ++ii) {
        char* pStr = allTmpImports->data[ii];

        const int bufSize = strlen(pStr) + 32;
        char* tmpBuf = calloc(bufSize, sizeof(char));
        snprintf(tmpBuf, bufSize, "#include %s\n", pStr);
        StreamWriter__write(streamWriter, tmpBuf, strlen(tmpBuf));
        free(tmpBuf);
      }

    }


    for (unsigned int ii = 0; ii < allStructs->len; ++ii) {
      char* pStr = allStructs->data[ii];
      int bufSize = strlen(pStr) + 5;
      char* tmpBuf = calloc(bufSize, sizeof(char));

      snprintf(tmpBuf, bufSize, "%s;\n\n", pStr);
      StreamWriter__write(streamWriter, tmpBuf, strlen(tmpBuf));

      free(tmpBuf);
    }

    char buffer[1024];
    memset(buffer, 0, 1024);

    for (unsigned int ii = 0; ii < allSignatures->len; ++ii) {
      char* pStr = allSignatures->data[ii];
      snprintf(buffer, 1024, "%s;\n\n", pStr);
      StreamWriter__write(streamWriter, buffer, strlen(buffer));
    }

    StreamWriter__free(&streamWriter);

    HashSet__set(self->allGeneratedFiles, outHeaderFilepath);
  }

  //
  //
  //


  ///MARK: text-replace
  {
    // apply the comptime comment text-replace
    HashSet* tmpSet = HashMap__get(self->allExtraIncludesPerFiles, filepath);
    if (tmpSet && HashSet__get_totalItems(tmpSet) > 0)
    {
      // char* outFolderPath = Path__dirname(filepath);

      printf(" GOT GENERATED STUFF: %s\n", filepath);

      unsigned int totalImports = 0;
      char** allImports = HashSet__get_allKeys(tmpSet, &totalImports);

      for (unsigned int importIndex = 0; importIndex < totalImports; ++importIndex)
      {
        const char* currImport = allImports[importIndex];

        printf(" -> currImport=%s\n", currImport);

        char* lcFilepath = NULL;
        char* extname = Path__extname(currImport);
        printf(" -> extname=%s\n", extname);
        if (strcmp(extname, ".h") == 0) {

          const int bufSize = strlen(currImport) + 5;
          lcFilepath = calloc(bufSize, sizeof(char));

          char* tmpStr = strndup(currImport, strlen(currImport) - 2);

          snprintf(lcFilepath, bufSize, "%s.lc", tmpStr);

          free(tmpStr);

          printf(" -> lcFilepath=%s\n", lcFilepath);
        }
        free(extname);

        char* tmpFilepath = NULL;
        if (lcFilepath) {
          tmpFilepath = lcFilepath;
        } else {
          tmpFilepath = strdup(currImport);
        }

        printf(" -> tmpFilepath=%s\n", tmpFilepath);

        const AnalyzedFile* importedAnalyzed = SourceAnalyzer__getAnalyzed(self->analyzer, tmpFilepath);

        free(tmpFilepath);


        // importedAnalyzed->indexer

        if (importedAnalyzed)
        {
          // ex: "HeapArena<int>" -> "HeapArena__int"
          const HashMap* textsToReplace = SourceIndexer__getComptimeTextsToReplace(importedAnalyzed->indexer);

          unsigned int totalItems = HashMap__get_totalItems(textsToReplace);
          if (totalItems > 0)
          {

            printf(" ----> totalItems=%d\n", totalItems);

            // update the C file
            {
              char* pFileContent;
              unsigned int fileSize;
              // TODO: this is slow
              if (readFile(outFilepath, &pFileContent, &fileSize) < 0)
              {
                // TODO: need a goto to a failure_return
                fprintf(stderr, "could not read a C file -> %s\n", outFilepath);
                panic("could not read a C file");
              }

              int inputAllocated = 0;
              char* inputBuffer = pFileContent;
              unsigned int inputLen = fileSize;

              unsigned int totalItems = 0;
              EntryItem* allItems = HashMap__get_allItems(textsToReplace, &totalItems);
              for (unsigned int ii = 0; ii < totalItems; ++ii)
              {
                const char* srcText = allItems[ii].key;
                const char* dstText = allItems[ii].value;
                StringData result = String__replaceAll2(inputBuffer, srcText, dstText);

                if (inputAllocated != 0) {
                  free(inputBuffer);
                }
                inputAllocated = 1;
                inputBuffer = result.data;
                inputLen = result.len;
              }

              free(allItems);

              // TODO: this is slow
              if (writeFile(outFilepath, inputBuffer, inputLen) < 0)
              {
                // TODO: need a goto to a failure_return
                fprintf(stderr, "could not write a C file -> %s\n", outFilepath);
                panic("could not write a C file");
              }

              free(inputBuffer);
              free(pFileContent);
            }

            // update the H file (if any)
            if (needHeaderFile != 0)
            {
              char* pFileContent;
              unsigned int fileSize;
              // TODO: this is slow
              if (readFile(outHeaderFilepath, &pFileContent, &fileSize) < 0)
              {
                // TODO: need a goto to a failure_return
                fprintf(stderr, "could not read a H file -> %s\n", outHeaderFilepath);
                panic("could not read a H file");
              }

              int inputAllocated = 0;
              char* inputBuffer = pFileContent;
              unsigned int inputLen = fileSize;

              unsigned int totalItems = 0;
              EntryItem* allItems = HashMap__get_allItems(textsToReplace, &totalItems);
              for (unsigned int ii = 0; ii < totalItems; ++ii)
              {
                const char* srcText = allItems[ii].key;
                const char* dstText = allItems[ii].value;
                StringData result = String__replaceAll2(inputBuffer, srcText, dstText);

                if (inputAllocated != 0) {
                  free(inputBuffer);
                }
                inputAllocated = 1;
                inputBuffer = result.data;
                inputLen = result.len;
              }

              free(allItems);

              // TODO: this is slow
              if (writeFile(outHeaderFilepath, inputBuffer, inputLen) < 0)
              {
                // TODO: need a goto to a failure_return
                fprintf(stderr, "could not write a H file -> %s\n", outHeaderFilepath);
                panic("could not write a H file");
              }

              free(inputBuffer);
              free(pFileContent);
            }


          }
        }

      }

      free(allImports);

    }


  }


  //
  //
  //

  ///MARK: cleanup

  for (unsigned int ii = 0; ii < allEdits->len; ++ii) {
    AnyEdit* currEdit = allEdits->data[ii];
    switch (currEdit->editType)
    {
    case ET_CONVERT_RANGE:
      free(currEdit->editValue.convertRange.replacement);
      break;
    case ET_COMMENT_RANGE:
    default:
      break;
    }
    free(currEdit);
  }
  PointerHeapArray__free(&allEdits);

  for (unsigned int ii = 0; ii < allTmpImports->len; ++ii) {
    char* pStr = allTmpImports->data[ii];
    free(pStr);
  }
  PointerHeapArray__free(&allTmpImports);

  for (unsigned int ii = 0; ii < allStructs->len; ++ii) {
    char* pStr = allStructs->data[ii];
    free(pStr);
  }
  PointerHeapArray__free(&allStructs);

  for (unsigned int ii = 0; ii < allSignatures->len; ++ii) {
    char* pStr = allSignatures->data[ii];
    free(pStr);
  }
  PointerHeapArray__free(&allSignatures);

  QueryMatchData__free(&newMatchData);
  free(outHeaderFilepath);
  free(outFilepath);
  free(inputFilepathExt);
  free(outSrcFolder);

  StopWatch__stop(&stopWatch);
  const double timeInSec = StopWatch__getTime(&stopWatch);
  printf("   -> %lf sec\n", timeInSec);
  StopWatch__free(&stopWatch);

  return 0;
}

//MARK: getOutputSource
PointerHeapArray* Transpiler__getOutputSource(const Transpiler* self)
{
  unsigned int totalAnalyzed;
  char** allAnalyzed = HashSet__get_allKeys(self->allGeneratedFiles, &totalAnalyzed);

  printf(" -> totalAnalyzed:%d\n", totalAnalyzed);

  PointerHeapArray* sourcesFilepaths = PointerHeapArray__preAllocate(32);
  if (!sourcesFilepaths)
  {
    panic("OOM?");
  }

  for (unsigned int ii = 0; ii < totalAnalyzed; ++ii) {

    char* currFilepath = allAnalyzed[ii];

    // must exclude the comptime main file
    if (strstr(currFilepath, "/.generated/comptime.main.c") != NULL)
    {
      continue;
    }

    {
      char* currExt = Path__extname(currFilepath);
      int isHeaderFile = (strcmp(currExt, ".h") == 0) ? 1 : 0;
      free(currExt);

      // must skip the header files
      if (isHeaderFile == 1)
      {
        // printf(" -> SKIPPED: %s\n", currFilepath);
        continue;
      }
    }

    // {
    //   char* currExt = Path__extname(currFilepath);
    //   int isLazyC = (strcmp(currExt, ".lc") == 0) ? 1 : 0;
    //   free(currExt);

    //   // must skip the lazy-c files
    //   if (isLazyC == 1)
    //   {
    //     printf(" -> SKIPPED: %s\n", currFilepath);
    //     continue;
    //   }
    // }

    printf(" ---> to compile: %s\n", currFilepath);

    if (PointerHeapArray__pushBack(sourcesFilepaths, currFilepath) != 0)
    {
      panic("OOM?");
    }
  }


  return sourcesFilepaths;
}