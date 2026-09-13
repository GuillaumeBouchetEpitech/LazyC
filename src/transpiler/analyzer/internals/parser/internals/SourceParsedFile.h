
#pragma once

#include "stdlib/collections/HashMap.h"
#include "stdlib/strings/StringData.h"
#include "stdlib/filesystem/StreamWriter.h"

// forward declaration
typedef struct TSTree TSTree;
typedef struct TSQuery TSQuery;

typedef struct NodePos {
  unsigned int row;
  unsigned int column;
  unsigned int index;
} NodePos;

typedef struct SourceParsedFile
{
  StringData fileData;
  TSTree *pTree;

  NodePos startPos;
  NodePos endPos;

} SourceParsedFile;

typedef struct NodeData {
  NodePos startPos;
  NodePos endPos;
  StringData captureName;
} NodeData;

typedef struct QueryMatchData {
  NodeData* allNodes;
  unsigned int totalNodes;
  HashMap* captureMap;
  struct QueryMatchData* next;
} QueryMatchData;

void SourceParsedFile__free(SourceParsedFile** inParsedFile);
const char* SourceParsedFile__getFileContent(const SourceParsedFile* inParsedFile);
void SourceParsedFile__debugTree(const SourceParsedFile* inParsedFile, StreamWriter* streamWriter);

NodePos SourceParsedFile__getStartPos(SourceParsedFile *self);
NodePos SourceParsedFile__getEndPos(SourceParsedFile *self);

// QueryMatchData* SourceParsedFile__query(const SourceParsedFile* inParsedFile, const char* inQueryData, unsigned int inQueryLength);
QueryMatchData *SourceParsedFile__query(const SourceParsedFile *inParsedFile, TSQuery *inQuery);
void QueryMatchData__free(QueryMatchData** self);


