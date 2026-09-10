
#include "./SourceAnalyzer.h"

#include "./internals/parser/SourceParser.h"

#include "stdlib/core/panic.h"
#include "stdlib/collections/HashMap.h"
#include "stdlib/collections/HashSet.h"
#include "stdlib/collections/PointerHeapArray.h"
// #include "stdlib/filesystem/readFile.h"
#include "stdlib/filesystem/pathUtils.h"
#include "stdlib/filesystem/statUtils.h"

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct SourceAnalyzer
{
  SourceParser *parser;
  HashMap *filesMap; // ==> [filepath]: AnalyzedFile*
  HashSet *scannedFoldersSet; // ==> [folderPath]
  HashSet *nextFoldersToExploreSet; // ==> [folderPath]
  HashSet *nextFileToScanSet; // ==> [filepath]
  HashMap *definitionToFileMap; // ==> [definition]: filepath

  // PointerHeapArray<AnalyzedFile>
  PointerHeapArray* depSortedAnalyzed;

} SourceAnalyzer;


// forward declaration(s)
AnalyzedFile *AnalyzedFile__create(SourceParser *inParser, const char *inFilepath);
void AnalyzedFile__free(AnalyzedFile **self);

// MARK: create
SourceAnalyzer *SourceAnalyzer__create()
{
  SourceAnalyzer *newAnalyzer = calloc(1, sizeof(SourceAnalyzer));
  if (!newAnalyzer)
  {
    return NULL;
  }

  newAnalyzer->parser = SourceParser__create();
  newAnalyzer->filesMap = HashMap__preAllocate(32);
  newAnalyzer->scannedFoldersSet = HashSet__preAllocate(32);
  newAnalyzer->nextFoldersToExploreSet = HashSet__preAllocate(32);
  newAnalyzer->nextFileToScanSet = HashSet__preAllocate(32);
  newAnalyzer->definitionToFileMap = HashMap__preAllocate(32);
  newAnalyzer->depSortedAnalyzed = PointerHeapArray__preAllocate(32);

  if (!newAnalyzer->parser ||
      !newAnalyzer->filesMap ||
      !newAnalyzer->scannedFoldersSet ||
      !newAnalyzer->nextFileToScanSet ||
      !newAnalyzer->definitionToFileMap ||
      !newAnalyzer->depSortedAnalyzed)
  {
    SourceAnalyzer__free(&newAnalyzer);
    return NULL;
  }

  return newAnalyzer;
}

// MARK: free
void SourceAnalyzer__free(SourceAnalyzer **self)
{
  if (!self || !*self)
  {
    return;
  }


  if ((*self)->parser)
  {
    SourceParser__free(&(*self)->parser);
  }
  if ((*self)->filesMap)
  {

    {
      unsigned int totalKeys;
      EntryItem* allItems = HashMap__get_allItems((*self)->filesMap, &totalKeys);

      for (unsigned int ii = 0; ii < totalKeys; ++ii)
      {
        AnalyzedFile* currFile = allItems[ii].value;
        AnalyzedFile__free(&currFile);
      }

      // free(allKeys);
      free(allItems);
    }

    HashMap__free(&(*self)->filesMap);
  }
  if ((*self)->nextFoldersToExploreSet)
  {
    HashSet__free(&(*self)->nextFoldersToExploreSet);
  }
  if ((*self)->scannedFoldersSet)
  {
    HashSet__free(&(*self)->scannedFoldersSet);
  }
  if ((*self)->nextFileToScanSet)
  {
    HashSet__free(&(*self)->nextFileToScanSet);
  }
  if ((*self)->definitionToFileMap)
  {
    HashMap__free(&(*self)->definitionToFileMap);
  }
  if ((*self)->depSortedAnalyzed)
  {
    PointerHeapArray__free(&(*self)->depSortedAnalyzed);
  }
  free(*self);
  *self = NULL;
}

// forward declaration
void SourceAnalyzer__exploreFolder(SourceAnalyzer *self, const char *inFolderPath);
// int SourceAnalyzer__scanFile(SourceAnalyzer *self, const char *inFilepath);


static int _sortByImportedDepCallback(void *left, void *right, void *userData)
{
  (void)userData; // unused

  const AnalyzedFile * leftData = left;
  const AnalyzedFile * rightData = right;

  unsigned int leftTotal = SourceIndexer__getTotalImports(leftData->indexer);
  unsigned int rightTotal = SourceIndexer__getTotalImports(rightData->indexer);

  // const HashSet* leftComptimeResolve = SourceIndexer__getComptimeTypesToResolve(leftData->indexer);
  // const HashSet* rightComptimeResolve = SourceIndexer__getComptimeTypesToResolve(rightData->indexer);
  // leftTotal += HashSet__get_totalItems(leftComptimeResolve);
  // rightTotal += HashSet__get_totalItems(rightComptimeResolve);

  // printf("--> left:  %d\n--> right: %d\n", HashSet__get_totalItems(leftComptimeResolve), HashSet__get_totalItems(rightComptimeResolve));

  if (leftTotal == 0 && rightTotal == 0) {
    return 0;
  }
  if (leftTotal == 0 && rightTotal != 0) {
    return 1;
  }
  if (leftTotal != 0 && rightTotal == 0) {
    return -1;
  }

  const int leftHasRight = SourceIndexer__hasImport(leftData->indexer, rightData->filepath);
  const int rightHasLeft = SourceIndexer__hasImport(rightData->indexer, leftData->filepath);

  if (leftHasRight && rightHasLeft) {
    // return 0; // <- circular dependency
    fprintf(stderr, "\n\nCIRCULAR DEPENDENCY\n--> left:  %s\n--> right: %s\n", leftData->filepath, rightData->filepath);
    panic("CIRCULAR DEPENDENCY");
  }
  if (leftHasRight && !rightHasLeft) {
    return -1;
  }
  if (!leftHasRight && rightHasLeft) {
    return 1;
  }
  return 0;
}

// MARK: scanFromMainFile
void SourceAnalyzer__scanFromMainFile(SourceAnalyzer *self, const char *inEntryFilepath, PointerHeapArray* inIncludePath)
{
  HashSet__set(self->nextFileToScanSet, inEntryFilepath);

  // dijkstra type of exploration
  PointerHeapArray* reusedListStr = PointerHeapArray__preAllocate(32);
  while (HashSet__get_totalItems(self->nextFileToScanSet) > 0)
  {
    // scan the files
    {
      // accumulate the files to scan
      {
        unsigned int totalFileToScans;
        char** allFilepathToScan = HashSet__get_allKeys(self->nextFileToScanSet, &totalFileToScans);

        for (unsigned int ii = 0; ii < totalFileToScans; ++ii)
        {
          const char* nextFilepath = allFilepathToScan[ii];
          PointerHeapArray__pushBack(reusedListStr, strdup(nextFilepath));
        }

        free(allFilepathToScan);
      }

      // scan the accumulated files
      for (unsigned int ii = 0; ii < reusedListStr->len; ++ii)
      {
        char* nextFilepath = reusedListStr->data[ii];

        SourceAnalyzer__scanFile(self, nextFilepath, inIncludePath);

        free(nextFilepath);
      }
      PointerHeapArray__clear(reusedListStr);
    }

    // explore the folders
    if (HashSet__get_totalItems(self->nextFoldersToExploreSet) > 0)
    {
      // accumulate the folders to explore
      {
        unsigned int totalKeys = 0;
        char** allKeys = HashSet__get_allKeys(self->nextFoldersToExploreSet, &totalKeys);

        for (unsigned int ii = 0; ii < totalKeys; ++ii)
        {
          PointerHeapArray__pushBack(reusedListStr, strdup(allKeys[ii]));
        }

        free(allKeys);
        HashSet__clear(self->nextFoldersToExploreSet);
      }

      // explore the accumulated folders
      for (unsigned int ii = 0; ii < reusedListStr->len; ++ii)
      {
        char* nextFolderPath = reusedListStr->data[ii];

        SourceAnalyzer__exploreFolder(self, nextFolderPath);

        free(nextFolderPath);
      }
      PointerHeapArray__clear(reusedListStr);
    }
  }

  PointerHeapArray__free(&reusedListStr);


  {
    // fill the container
    {
      unsigned int totalKeys;
      EntryItem* allItems = HashMap__get_allItems(self->filesMap, &totalKeys);

      for (unsigned int ii = 0; ii < totalKeys; ++ii)
      {
        PointerHeapArray__pushBack(self->depSortedAnalyzed, allItems[ii].value);
      }

      // free(allKeys);
      free(allItems);
    }

    // sort the container
    PointerHeapArray__sortFromCallback(self->depSortedAnalyzed, &_sortByImportedDepCallback, NULL);
  }

}

// MARK: exploreFolder
void SourceAnalyzer__exploreFolder(SourceAnalyzer *self, const char *inFolderPath)
{
  if (HashSet__contains(self->scannedFoldersSet, inFolderPath) != 0)
  {
    return;
  }
  HashSet__set(self->scannedFoldersSet, inFolderPath);

  printf("EXPLORE_FOLDER: %s\n", inFolderPath);

  DIR *currDir = opendir(inFolderPath);
  if (!currDir) {
    printf("EXPLORE_FOLDER FAILURE\n");
    perror("opendir");
    return;
  }
  struct dirent *dir;
  while ((dir = readdir(currDir)) != NULL)
  {
    if (strcmp(dir->d_name, ".") == 0)
    {
      continue;
    }

    if (strcmp(dir->d_name, ".generated") == 0)
    {
      char *folderPathToExplore = Path__join(2, inFolderPath, dir->d_name);
      HashSet__set(self->nextFoldersToExploreSet, folderPathToExplore);
      free(folderPathToExplore);
      continue;
    }

    char *extName = Path__extname(dir->d_name);
    const int mustSkip = (strcmp(extName, ".lc") != 0 &&
                          strcmp(extName, ".c") != 0 &&
                          strcmp(extName, ".h") != 0)
                             ? 1
                             : 0;
    free(extName);

    // printf("MAYBE FILE -> %s, mustSkip=%d\n", dir->d_name, mustSkip);

    if (mustSkip != 0)
    {
      continue;
    }

    char *filepathToScan = Path__join(2, inFolderPath, dir->d_name);

    if (HashMap__contains(self->filesMap, filepathToScan) == 0)
    {
      HashSet__set(self->nextFileToScanSet, filepathToScan);
    }

    // SourceAnalyzer__scanFile(self, filepathToScan);

    free(filepathToScan);
  }
  closedir(currDir);
}

// MARK: scanFile
int SourceAnalyzer__scanFile(SourceAnalyzer *self, const char *inFilepath, PointerHeapArray* inIncludePath)
{
  if (HashMap__contains(self->filesMap, inFilepath) != 0)
  {
    return 0;
  }

  HashSet__remove(self->nextFileToScanSet, inFilepath);

  {
    if (
      strstr(inFilepath, ".tests.c") != NULL ||
      strstr(inFilepath, ".tests.h") != NULL ||
      strstr(inFilepath, ".tests.lc") != NULL
    ) {
      return 0;
    }
  }

  printf("SCANFILE: %s\n", inFilepath);

  AnalyzedFile *analyzedFile = AnalyzedFile__create(self->parser, inFilepath);

  // if (!analyzedFile)
  // {
    // if (inIncludePath)
    // {
    //   for (unsigned int ii = 0; ii < inIncludePath->len; ++ii)
    //   {
    //     const char* currFolderPath = inIncludePath->data[ii];

    //     char* tmpPath = Path__join(2, currFolderPath, inFilepath);
    //     analyzedFile = AnalyzedFile__create(self->parser, tmpPath);
    //     free(tmpPath);

    //     if (analyzedFile) {
    //       break;
    //     }
    //   }
    // }

    if (!analyzedFile)
    {
      fprintf(stderr, "FAILED TO ANALYZE A FILE! -> %s\n", inFilepath);
      return -1;
    }
  // }

  // even if failed/NULL -> must record it in the HashMap
  HashMap__set(self->filesMap, inFilepath, analyzedFile);

  // if (!analyzedFile)
  // {
  //   fprintf(stderr, "FAILED TO ANALYZE A FILE! -> %s\n", inFilepath);
  //   return -1;
  // }

  char* currDir = Path__dirname(analyzedFile->filepath);

  {
    // check if the file's folder also need to be explored
    if (HashSet__contains(self->scannedFoldersSet, currDir) == 0)
    {
      HashSet__set(self->nextFoldersToExploreSet, currDir);
    }
  }

  unsigned int totalImports;
  char** allImports = SourceIndexer__getAllImports(analyzedFile->indexer, &totalImports);

  // printf(" = = > totalImports=%d\n", totalImports);

  for (unsigned int ii = 0; ii < totalImports; ++ii)
  {
    // char* fullPath = Path__join(2, currDir, allImports[ii]);
    char* fullPath = allImports[ii];

    // printf(" = = = > %s\n", fullPath);

    if (HashMap__contains(self->filesMap, fullPath) == 0)
    {
      HashSet__set(self->nextFileToScanSet, fullPath);
    }

    // free(fullPath);
  }

  free(allImports);
  free(currDir);

  if (inIncludePath)
  {
    unsigned int totalRawImports;
    char** allRawImports = SourceIndexer__getAllRawImports(analyzedFile->indexer, &totalRawImports);

    for (unsigned int ii = 0; ii < totalRawImports; ++ii)
    {
      char* currRawImport = allRawImports[ii];

      // is absolute or relative -> skip
      if (currRawImport[0] == '/' || currRawImport[0] == '.') {
        continue;
      }

      // char* fullPath = Path__join(2, currDir, allImports[ii]);

      for (unsigned int jj = 0; jj < inIncludePath->len; ++jj)
      {
        const char* currFolderPath = inIncludePath->data[jj];

        char* tmpPath = Path__join(2, currFolderPath, currRawImport);

        if (HashMap__contains(self->filesMap, tmpPath) == 0 && Stat__pathExist(tmpPath))
        {
          HashSet__set(self->nextFileToScanSet, tmpPath);
          free(tmpPath);
          break;
        }

        free(tmpPath);
      }

    }

    free(allRawImports);
  }

  {
    if (
      analyzedFile->fileType == SOURCE_H ||
      analyzedFile->fileType == SOURCE_LC
    ) {

      const HashSet* exportedDefSet = SourceIndexer__getExportedDef(analyzedFile->indexer);

      if (HashSet__get_totalItems(exportedDefSet) > 0)
      {
        unsigned int totalKeys = 0;
        char** allKeys = HashSet__get_allKeys(exportedDefSet, &totalKeys);

        for (unsigned int ii = 0; ii < totalKeys; ++ii)
        {
          // printf("set definitionToFileMap: [%s] -> %s\n", allKeys[ii], analyzedFile->filepath);
          HashMap__set(self->definitionToFileMap, allKeys[ii], analyzedFile);
        }

        free(allKeys);
      }
    }
  }


  return 0;
}

//MARK: getAllAnalyzed
char **SourceAnalyzer__getAllAnalyzed(const SourceAnalyzer *self, unsigned int *outTotalAnalyzed)
{
  return HashMap__get_allKeys(self->filesMap, outTotalAnalyzed);
}

//MARK: getAnalyzed
const AnalyzedFile *SourceAnalyzer__getAnalyzed(const SourceAnalyzer *self, const char *inFilepath)
{
  return HashMap__get(self->filesMap, inFilepath);
}

//MARK: getAnalyzedFromDefinitionName
const AnalyzedFile *SourceAnalyzer__getAnalyzedFromDefinitionName(const SourceAnalyzer *self, const char *inDefinitionName)
{
  return HashMap__get(self->definitionToFileMap, inDefinitionName);
}

// MARK: getSortedAnalyzedFiles
// PointerHeapArray<AnalyzedFile>
const PointerHeapArray *SourceAnalyzer__getSortedAnalyzedFiles(const SourceAnalyzer *self)
{
  return self->depSortedAnalyzed;
}

