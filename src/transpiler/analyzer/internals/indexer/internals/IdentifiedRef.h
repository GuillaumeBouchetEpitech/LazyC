
#pragma once

#include "../../parser/SourceParser.h"

// forward declaration
typedef struct SourceScope SourceScope;

typedef struct IdentifiedRef
{
  SourceScope *parentScope;
  NodePos startPos;
  NodePos endPos;
  char *varName;
}
IdentifiedRef;

IdentifiedRef IdentifiedRef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName);

void IdentifiedRef__free(IdentifiedRef *self);
