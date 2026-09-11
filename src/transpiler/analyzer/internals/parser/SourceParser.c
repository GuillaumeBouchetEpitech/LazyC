
#include "./SourceParser.h"

#include "stdlib/core/panic.h"
#include "stdlib/filesystem/readFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-lazyc.h>

typedef struct SourceParser
{
  TSParser *pParser;
} SourceParser;

SourceParser *SourceParser__create()
{
  SourceParser *newSrcParser = calloc(1, sizeof(SourceParser));
  if (!newSrcParser)
  {
    return NULL;
  }

  newSrcParser->pParser = ts_parser_new();
  if (
    !newSrcParser->pParser ||
    !ts_parser_set_language(newSrcParser->pParser, tree_sitter_lazyc())
  ) {
    fprintf(stderr, "grammar/runtime ABI mismatch\n");
    SourceParser__free(&newSrcParser);
    return NULL;
  }

  return newSrcParser;
}

void SourceParser__free(SourceParser **self)
{
  if (!self || !*self)
  {
    return;
  }

  // TODO: free all parsed files here?

  if ((*self)->pParser) {
    ts_parser_delete((*self)->pParser);
    (*self)->pParser = NULL;
  }
  free(*self);
  *self = NULL;
}

SourceParsedFile *SourceParser__parse(SourceParser *sefl, const char *inFilepath)
{
  StringData fileData;
  if (readFile(inFilepath, &fileData.data, &fileData.len) < 0)
  {
    return NULL;
  }

  TSTree *pTree = ts_parser_parse_string(sefl->pParser, NULL, fileData.data, (uint32_t)fileData.len);
  if (!pTree)
  {
    return NULL;
  }

  SourceParsedFile *newParsedFile = calloc(1, sizeof(SourceParsedFile));
  if (!newParsedFile)
  {
    return NULL;
  }

  newParsedFile->fileData.data = fileData.data;
  newParsedFile->fileData.len = fileData.len;
  newParsedFile->pTree = pTree;

  TSNode rootNode = ts_tree_root_node(pTree);
  const uint32_t startIndex = ts_node_start_byte(rootNode);
  const uint32_t endIndex = ts_node_end_byte(rootNode);
  const TSPoint startPoint = ts_node_start_point(rootNode);
  const TSPoint endPoint = ts_node_end_point(rootNode);

  newParsedFile->startPos.index = startIndex;
  newParsedFile->startPos.row = startPoint.row;
  newParsedFile->startPos.column = startPoint.column;

  newParsedFile->endPos.index = endIndex;
  newParsedFile->endPos.row = endPoint.row;
  newParsedFile->endPos.column = endPoint.column;

  return newParsedFile;
}
