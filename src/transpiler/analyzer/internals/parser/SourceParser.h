
#pragma once

#include "stdlib/collections/HashMap.h"
#include "stdlib/strings/StringData.h"
#include "stdlib/filesystem/StreamWriter.h"

typedef struct SourceParser SourceParser;
typedef struct SourceParsedFile SourceParsedFile;

typedef struct NodePos {
  unsigned int row;
  unsigned int column;
  unsigned int index;
} NodePos;

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

SourceParser* SourceParser__create();
void SourceParser__free(SourceParser** inParser);
SourceParsedFile* SourceParser__parse(SourceParser* inParser, const char* inFilepath);
void SourceParsedFile__free(SourceParsedFile** inParsedFile);
const char* SourceParsedFile__getFileContent(const SourceParsedFile* inParsedFile);
void SourceParsedFile__debugTree(const SourceParsedFile* inParsedFile, StreamWriter* streamWriter);

NodePos SourceParsedFile__getStartPos(SourceParsedFile *self);
NodePos SourceParsedFile__getEndPos(SourceParsedFile *self);

QueryMatchData* SourceParsedFile__query(const SourceParsedFile* inParsedFile, const char* inQueryData, unsigned int inQueryLength);
void QueryMatchData__free(QueryMatchData** self);

