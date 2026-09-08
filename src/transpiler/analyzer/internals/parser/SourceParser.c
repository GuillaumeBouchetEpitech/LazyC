
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

typedef struct SourceParsedFile
{
  StringData fileData;
  TSTree *pTree;

  NodePos startPos;
  NodePos endPos;

} SourceParsedFile;

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

void SourceParsedFile__free(SourceParsedFile **inParsedFile)
{
  if (*inParsedFile == NULL)
  {
    return;
  }
  free((*inParsedFile)->fileData.data);
  ts_tree_delete((*inParsedFile)->pTree);
  free(*inParsedFile);
  *inParsedFile = NULL;
}

static void _traverseWithTreeCursor(const char *inSource, StreamWriter *inStreamWriter, TSTreeCursor *pTreeCursor, int inLevel)
{
  do
  {
    // TSNode ts_tree_cursor_current_node(const TSTreeCursor *);
    // const char *ts_tree_cursor_current_field_name(const TSTreeCursor *);
    // TSFieldId ts_tree_cursor_current_field_id(const TSTreeCursor *);

    const TSNode currNode = ts_tree_cursor_current_node(pTreeCursor);

    const char *pNodeType = ts_node_type(currNode);

    const uint32_t startIndex = ts_node_start_byte(currNode);
    const uint32_t endIndex = ts_node_end_byte(currNode);
    const TSPoint startPoint = ts_node_start_point(currNode);
    const TSPoint endPoint = ts_node_end_point(currNode);

    char buffer[1024];
    memset(buffer, 0, 1024);
    for (int ii = 0; ii < inLevel; ++ii)
    {
      snprintf(buffer, 1024, "|---");
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    snprintf(buffer, 1024, " [L%d_C%d]->[L%d_C%d] ", startPoint.row, startPoint.column, endPoint.row, endPoint.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    if (ts_node_is_named(currNode))
    {
      snprintf(buffer, 1024, "[%s]", pNodeType);
    }
    else
    {
      snprintf(buffer, 1024, "\"%s\"", pNodeType);
    }
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " ---> \"%.*s\"", (int)(endIndex - startIndex), inSource + startIndex);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    // StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    if (ts_tree_cursor_goto_first_child(pTreeCursor))
    {
      _traverseWithTreeCursor(inSource, inStreamWriter, pTreeCursor, inLevel + 1);
      ts_tree_cursor_goto_parent(pTreeCursor);
    }
  } while (ts_tree_cursor_goto_next_sibling(pTreeCursor));
}

const char *SourceParsedFile__getFileContent(const SourceParsedFile *inParsedFile)
{
  return inParsedFile->fileData.data;
}

void SourceParsedFile__debugTree(const SourceParsedFile *inParsedFile, StreamWriter *streamWriter)
{
  TSNode rootNode = ts_tree_root_node(inParsedFile->pTree);
  TSTreeCursor treeCursor = ts_tree_cursor_new(rootNode);
  _traverseWithTreeCursor(inParsedFile->fileData.data, streamWriter, &treeCursor, 1);
  ts_tree_cursor_delete(&treeCursor);
}

NodePos SourceParsedFile__getStartPos(SourceParsedFile *self)
{
  return self->startPos;
}

NodePos SourceParsedFile__getEndPos(SourceParsedFile *self)
{
  return self->endPos;
}

static QueryMatchData *_QueryMatchData_create(TSQuery *inQuery, TSQueryMatch *inMatch);

QueryMatchData *SourceParsedFile__query(const SourceParsedFile *inParsedFile, const char *inQueryData, unsigned int inQueryLength)
{

  uint32_t err_offset;
  TSQueryError err_type;
  TSQuery *query = ts_query_new(tree_sitter_lazyc(), inQueryData, (uint32_t)inQueryLength, &err_offset, &err_type);
  if (!query)
  {
    fprintf(stderr, "bad query (type %d) at byte %u\n", err_type, err_offset);
    panic("bad tree-sitter query");
    // return NULL;
  }

  TSQueryCursor *cursor = ts_query_cursor_new();
  TSNode rootNode = ts_tree_root_node(inParsedFile->pTree);
  ts_query_cursor_exec(cursor, query, rootNode);

  QueryMatchData *rootMatch = NULL;
  QueryMatchData *lastMatch = NULL;
  TSQueryMatch match;
  while (ts_query_cursor_next_match(cursor, &match))
  {
    // printf("-> cursor.id:%d\n", match.id);
    // printf("-> %s\n", ts_node_string(match.captures->node));
    // int nameLen;
    // printf("---> %s\n", ts_query_capture_name_for_id(query, match.id, &nameLen));

    QueryMatchData *newMatchData = _QueryMatchData_create(query, &match);

    if (!rootMatch)
    {
      rootMatch = newMatchData;
    }
    if (lastMatch)
    {
      lastMatch->next = newMatchData;
    }
    lastMatch = newMatchData;
  }

  ts_query_cursor_delete(cursor);
  ts_query_delete(query);

  return rootMatch;
}

//
//
//

//
//
//

//
//
//

static QueryMatchData *_QueryMatchData_create(TSQuery *inQuery, TSQueryMatch *inMatch)
{

  QueryMatchData *newMatchData = calloc(1, sizeof(QueryMatchData));
  if (!newMatchData)
  {
    return NULL;
  }

  newMatchData->allNodes = calloc(inMatch->capture_count, sizeof(NodeData));
  if (!newMatchData->allNodes)
  {
    free(newMatchData);
    return NULL;
  }
  newMatchData->totalNodes = inMatch->capture_count;
  newMatchData->next = NULL;

  newMatchData->captureMap = HashMap__preAllocate(32);
  if (!newMatchData->captureMap) {
    free(newMatchData);
    return NULL;
  }

  for (uint16_t ii = 0; ii < inMatch->capture_count; ++ii)
  {
    TSQueryCapture capture = inMatch->captures[ii];
    // TSNode node = capture.node;

    const uint32_t startIndex = ts_node_start_byte(capture.node);
    const uint32_t endIndex = ts_node_end_byte(capture.node);
    const TSPoint sp = ts_node_start_point(capture.node);
    const TSPoint ep = ts_node_end_point(capture.node);

    NodeData *currNode = &newMatchData->allNodes[ii];
    currNode->startPos.index = startIndex;
    currNode->startPos.row = sp.row;
    currNode->startPos.column = sp.column;
    currNode->endPos.index = endIndex;
    currNode->endPos.row = ep.row;
    currNode->endPos.column = ep.column;

    uint32_t nameLen;
    const char *cap_name = ts_query_capture_name_for_id(inQuery, capture.index, &nameLen);
    if (!cap_name) {
      return NULL;
    }

    // printf(" -> cap_name[%d]: %s\n", nameLen, cap_name);

    // duplicate the capture name
    currNode->captureName.len = nameLen;
    currNode->captureName.data = strndup(cap_name, nameLen);
    if (!currNode->captureName.data) {
      return NULL;
    }

    // printf(" -> cap_name[%d]: %s\n", nameLen, currNode.captureName.data);

    // char *cap_name = ts_node_string(capture.node);
    // currNode.captureName.data = strdup(cap_name);
    // currNode.captureName.len = strlen(cap_name);

    // record the node in the hasmap
    if (HashMap__set(newMatchData->captureMap, currNode->captureName.data, currNode) < 0) {
      return NULL;
    }
  }

  return newMatchData;
}

void QueryMatchData__free(QueryMatchData **self)
{
  if (!self || !*self)
  {
    return;
  }

  QueryMatchData *currMatchData = *self;
  while (currMatchData)
  {
    QueryMatchData *nextMatchData = currMatchData->next;

    for (unsigned int ii = 0; ii < currMatchData->totalNodes; ++ii)
    {
      free(currMatchData->allNodes[ii].captureName.data);
    }
    free(currMatchData->allNodes);
    HashMap__free(&currMatchData->captureMap);
    free(currMatchData);

    currMatchData = nextMatchData;
  }

  *self = NULL;
}
