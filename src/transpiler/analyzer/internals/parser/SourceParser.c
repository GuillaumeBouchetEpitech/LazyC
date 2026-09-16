
#include "./SourceParser.h"

#include "stdlib/core/panic.h"
#include "stdlib/filesystem/readFile.h"
#include "stdlib/collections/HeapHashMap.lc"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-lazyc.h>

typedef struct SourceParser
{
  TSParser *pParser;
  HeapHashMap<TSQuery*> allQueries;
}
SourceParser;

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

  newSrcParser->allQueries = HeapHashMap<TSQuery*>::preAllocate(32);

  return newSrcParser;
}

void SourceParser__free(SourceParser **self)
{
  if (!self || !*self)
  {
    return;
  }

  {
    unsigned int totalItems;
    HeapHashMap<TSQuery*>__EntryItem* allItems = HeapHashMap<TSQuery*>::get_allItems(&(*self)->allQueries, &totalItems);
    for (unsigned int ii = 0; ii < totalItems; ++ii)
    {
      ts_query_delete(*allItems[ii].value);
    }
    free(allItems);

    HeapHashMap<TSQuery*>::free(&(*self)->allQueries);
  }

  if ((*self)->pParser) {
    ts_parser_delete((*self)->pParser);
    (*self)->pParser = NULL;
  }
  free(*self);
  *self = NULL;
}

SourceParsedFile *SourceParser__parse(SourceParser *self, const char *inFilepath)
{
  StringData fileData;
  if (readFile(inFilepath, &fileData.data, &fileData.len) < 0)
  {
    return NULL;
  }

  TSTree *pTree = ts_parser_parse_string(self->pParser, NULL, fileData.data, (uint32_t)fileData.len);
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

TSQuery* SourceParser__parseQuery(SourceParser *self, const char* inAlias, const char* inQueryStr)
{
  TSQuery** query = HeapHashMap<TSQuery*>::get(&self->allQueries, inAlias);
  if (query != NULL) {
    return *query;
  }

  uint32_t err_offset;
  TSQueryError err_type;
  TSQuery* newQuery = ts_query_new(tree_sitter_lazyc(), inQueryStr, (uint32_t)strlen(inQueryStr), &err_offset, &err_type);
  if (newQuery == NULL)
  {
    fprintf(stderr, "bad query (type %d) at byte %u\n", err_type, err_offset);
    fprintf(stderr, "-> \"%s\"\n", inQueryStr + err_offset);
    panic("bad tree-sitter query");
  }
  HeapHashMap<TSQuery*>::set(&self->allQueries, inAlias, newQuery);

  return newQuery;
}
