
#pragma once

#include "./internals/AnalyzedFile.h"
#include "./internals/indexer/SourceIndexer.h"

typedef struct AnalyzedFile AnalyzedFile;

typedef struct SourceAnalyzer SourceAnalyzer;

typedef struct SourceParser SourceParser;

typedef struct PointerHeapArray PointerHeapArray;

SourceAnalyzer *SourceAnalyzer__create(int handleTests);
void SourceAnalyzer__free(SourceAnalyzer **self);
void SourceAnalyzer__scanFromMainFile(SourceAnalyzer *self, const char *inEntryFilepath, PointerHeapArray* inIncludePath);
// void SourceAnalyzer__exploreFolder(SourceAnalyzer *self, const char *inFolderPath);
int SourceAnalyzer__scanFile(SourceAnalyzer *self, const char *inFilepath, PointerHeapArray* inIncludePath);

char **SourceAnalyzer__getAllAnalyzed(const SourceAnalyzer *self, unsigned int *outTotalAnalyzed);
const AnalyzedFile *SourceAnalyzer__getAnalyzed(const SourceAnalyzer *self, const char *inFilepath);

const AnalyzedFile *SourceAnalyzer__getAnalyzedFromDefinitionName(const SourceAnalyzer *self, const char *inDefinitionName);

// PointerHeapArray<AnalyzedFile>
const PointerHeapArray *SourceAnalyzer__getSortedAnalyzedFiles(const SourceAnalyzer *self);

SourceParser* SourceAnalyzer__getParser(SourceAnalyzer *self);


