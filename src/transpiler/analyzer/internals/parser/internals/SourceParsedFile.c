
#include "./SourceParsedFile.h"

#include "stdlib/core/panic.h"
#include "stdlib/filesystem/readFile.h"

#include "stdlib/time/StopWatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-lazyc.h>

///MARK: SourceParsedFile
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

QueryMatchData *SourceParsedFile__query(const SourceParsedFile *inParsedFile, TSQuery *inQuery)
{
  TSQueryCursor *cursor = ts_query_cursor_new();
  TSNode rootNode = ts_tree_root_node(inParsedFile->pTree);
  ts_query_cursor_exec(cursor, inQuery, rootNode);

  QueryMatchData *rootMatch = NULL;
  QueryMatchData *lastMatch = NULL;
  TSQueryMatch match;
  while (ts_query_cursor_next_match(cursor, &match))
  {
    QueryMatchData *newMatchData = _QueryMatchData_create(inQuery, &match);

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

///MARK: QueryMatchData
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

    // duplicate the capture name
    currNode->captureName.len = nameLen;
    currNode->captureName.data = strndup(cap_name, nameLen);
    if (!currNode->captureName.data) {
      return NULL;
    }

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
