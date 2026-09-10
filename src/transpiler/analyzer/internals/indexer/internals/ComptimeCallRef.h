
#pragma once

#include "../../parser/SourceParser.h"

// forward declaration
typedef struct PointerHeapArray PointerHeapArray;

typedef struct ComptimeCallRef
{
  struct SourceScope *parentScope;
  NodePos startPos;
  NodePos endPos;

  char *varName;
  char *argsValue;
  // PointerHeapArray<cstring>
  PointerHeapArray *argsList;

  char *signature;

} ComptimeCallRef;

ComptimeCallRef *ComptimeCallRef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName,
    const char *inArgsValue);

void ComptimeCallRef__free(ComptimeCallRef **self);
