
#pragma once

#include "indexer/SourceIndexer.h"


typedef enum AnalyzedFileType
{
  SOURCE_C,
  SOURCE_H,
  SOURCE_LC,
}
AnalyzedFileType;

typedef struct AnalyzedFile
{
  char *filepath;
  AnalyzedFileType fileType;
  SourceParsedFile *parsedFile;
  SourceIndexer *indexer;

  int isRelevant;
}
AnalyzedFile;


AnalyzedFile *AnalyzedFile__create(SourceParser *inParser, const char *inFilepath);
void AnalyzedFile__free(AnalyzedFile **self);


const char *AnalyzedFile__getFilepath(const AnalyzedFile *self);
const SourceIndexer *AnalyzedFile__getIndexer(const AnalyzedFile *self);
const SourceParsedFile* AnalyzedFile__getParsedFile(const AnalyzedFile *self);

void AnalyzedFile__debugScopeTree(const AnalyzedFile *self, const char* inBaseDir, StreamWriter *inStreamWriter);
void AnalyzedFile__debugTree(const AnalyzedFile *self, StreamWriter *streamWriter);

