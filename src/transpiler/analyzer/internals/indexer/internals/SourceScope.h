
#pragma once

#include "../../parser/SourceParser.h"

#include "./VarDef.h"
#include "./IdentifiedRef.h"
#include "stdlib/collections/HeapArray.lc"

typedef enum ScopeType
{
  ROOT_SCOPE,
  FUNC_SCOPE,
  BLOCK_SCOPE,
  STRUCT_SCOPE,
  ENUM_SCOPE,
  UNION_SCOPE,
  TYPEDEF_SCOPE,
  UNKNOWN_SCOPE
}
ScopeType;

typedef struct SourceScope
{
  char *funcName; // function only?
  struct SourceScope *parentScope;
  HeapArray<SourceScope*> allChildrenScopes;

  ScopeType scopeType;
  NodePos startPos;
  NodePos endPos;

  HeapArray<VarDef> allVarDef;
  HeapArray<IdentifiedRef> allFuncCalls;
  HeapArray<ComptimeCallRef> allComptimeCalls;
  HeapArray<IdentifiedRef> allVarRefs;
}
SourceScope;

SourceScope *SourceScope__create(NodePos inStartPos, NodePos inEndPos);
void SourceScope__free(SourceScope **self);
