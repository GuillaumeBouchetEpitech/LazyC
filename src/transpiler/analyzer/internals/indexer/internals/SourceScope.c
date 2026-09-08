
#include "SourceScope.h"

#include "./VarDef.h"
// #include "./IdentifiedRef.h"
// #include "./ComptimeCallRef.h"

// #include "stdlib/collections/HashSet.h"
// #include "stdlib/collections/HashMap.h"
#include "stdlib/collections/PointerHeapArray.h"
// #include "stdlib/filesystem/StreamWriter.h"

#include <stdlib.h>
// #include <stdio.h>
// #include <string.h>

SourceScope *SourceScope__create(NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *newScope = calloc(1, sizeof(SourceScope));
  if (!newScope)
  {
    return NULL;
  }

  newScope->allChildrenScopes = PointerHeapArray__preAllocate(32);
  newScope->allVarDef = PointerHeapArray__preAllocate(32);
  newScope->allFuncCalls = PointerHeapArray__preAllocate(32);
  newScope->allComptimeCalls = PointerHeapArray__preAllocate(32);
  newScope->allVarRefs = PointerHeapArray__preAllocate(32);
  newScope->allStructDef = PointerHeapArray__preAllocate(32);
  if (!newScope->allChildrenScopes ||
      !newScope->allVarDef ||
      !newScope->allFuncCalls ||
      !newScope->allComptimeCalls ||
      !newScope->allVarRefs ||
      !newScope->allStructDef)
  {
    SourceScope__free(&newScope);
    return NULL;
  }

  newScope->startPos = inStartPos;
  newScope->endPos = inEndPos;

  return newScope;
}

void SourceScope__free(SourceScope **self)
{
  if (!self || !*self)
  {
    return;
  }
  // if ((*self)->funcName)
  // {
  free((*self)->funcName);
  // }
  PointerHeapArray__free(&(*self)->allStructDef);
  PointerHeapArray__free(&(*self)->allVarDef);
  PointerHeapArray__free(&(*self)->allFuncCalls);
  PointerHeapArray__free(&(*self)->allComptimeCalls);
  PointerHeapArray__free(&(*self)->allVarRefs);
  PointerHeapArray__free(&(*self)->allChildrenScopes);
  free(*self);
  *self = NULL;
}
