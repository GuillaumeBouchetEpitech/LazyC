
#pragma once

#include "../../parser/SourceParser.h"

// forward declaration
typedef struct SourceScope SourceScope;

typedef struct VarDef
{
  SourceScope *parentScope;
  NodePos startPos;
  NodePos endPos;

  char *varName;
  char *typeName;
  int pointerLevel;
} VarDef;

VarDef *VarDef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName,
    const char *inTypeName,
    int inPointerLevel);

void VarDef__free(VarDef **self);