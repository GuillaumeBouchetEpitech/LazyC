
#pragma once

#include "../../parser/SourceParser.h"

#include "./VarDef.h"
#include "./IdentifiedRef.h"
#include "stdlib/collections/HeapArray.lc"
#include "stdlib/collections/PointerHeapArray.h"

typedef enum ScopeType {
  ROOT_SCOPE,
  FUNC_SCOPE,
  BLOCK_SCOPE
} ScopeType;

typedef struct SourceScope
{
  struct SourceScope *parentScope;
  PointerHeapArray *allChildrenScopes;
  ScopeType scopeType;
  NodePos startPos;
  NodePos endPos;

  // PointerHeapArray *allStructDef;
  HeapArray<VarDef> allVarDef;
  HeapArray<IdentifiedRef> allFuncCalls;
  HeapArray<ComptimeCallRef> allComptimeCalls;
  HeapArray<IdentifiedRef> allVarRefs;

  char *funcName; // function only?

} SourceScope;

SourceScope *SourceScope__create(NodePos inStartPos, NodePos inEndPos);
void SourceScope__free(SourceScope **self);
