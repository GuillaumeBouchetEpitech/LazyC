
#pragma once

#include "../../parser/SourceParser.h"

// #include "./VarDef.h"
// #include "stdlib/collections/HeapArray.lc"

// forward declaration
typedef struct PointerHeapArray PointerHeapArray;

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

  // HeapArray<VarDef>* allVarDef2;

  PointerHeapArray *allVarDef;    // <- TODO: must be sorted by startPos.index
  PointerHeapArray *allFuncCalls; // <- TODO: must be sorted by startPos.index
  PointerHeapArray *allComptimeCalls; // <- TODO: must be sorted by startPos.index
  PointerHeapArray *allVarRefs;   // <- TODO: must be sorted by startPos.index
  PointerHeapArray *allStructDef; // <- TODO: must be sorted by startPos.index

  char *funcName; // function only?

} SourceScope;

SourceScope *SourceScope__create(NodePos inStartPos, NodePos inEndPos);
void SourceScope__free(SourceScope **self);
