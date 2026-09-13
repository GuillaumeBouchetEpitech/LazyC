
#pragma once

#include "../../parser/SourceParser.h"

#include "stdlib/collections/HeapArray.lc"
#include "stdlib/strings/StringData.h"

typedef struct ComptimeCallRef
{
  struct SourceScope *parentScope;
  NodePos startPos;
  NodePos endPos;

  char *varName;
  char *argsValue;
  HeapArray<StringData> argsList;

  char *signature;

} ComptimeCallRef;

ComptimeCallRef ComptimeCallRef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName,
    const char *inArgsValue);

void ComptimeCallRef__free(ComptimeCallRef *self);
