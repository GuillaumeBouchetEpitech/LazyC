
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
  SourceScope *self = calloc(1, sizeof(SourceScope));
  if (!self)
  {
    return NULL;
  }

  // self->allStructDef = PointerHeapArray__preAllocate(32);
  self->allVarDef = HeapArray<VarDef>::preAllocated(32);
  self->allFuncCalls = HeapArray<IdentifiedRef>::preAllocated(32);
  self->allComptimeCalls = HeapArray<ComptimeCallRef>::preAllocated(32);
  self->allVarRefs = HeapArray<IdentifiedRef>::preAllocated(32);
  self->allChildrenScopes = PointerHeapArray__preAllocate(32);
  if (!self->allChildrenScopes)
  {
    SourceScope__free(&self);
    return NULL;
  }

  self->startPos = inStartPos;
  self->endPos = inEndPos;

  return self;
}

void SourceScope__free(SourceScope **self)
{
  if (!self || !*self)
  {
    return;
  }

  free((*self)->funcName);
  (*self)->funcName = NULL;

  // PointerHeapArray__free(&(*self)->allStructDef);
  HeapArray<VarDef>::free(&(*self)->allVarDef);
  HeapArray<IdentifiedRef>::free(&(*self)->allFuncCalls);
  HeapArray<ComptimeCallRef>::free(&(*self)->allComptimeCalls);
  HeapArray<IdentifiedRef>::free(&(*self)->allVarRefs);
  PointerHeapArray__free(&(*self)->allChildrenScopes);
  free(*self);
  *self = NULL;
}
