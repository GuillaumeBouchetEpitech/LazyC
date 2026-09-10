
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

  self->allChildrenScopes = PointerHeapArray__preAllocate(32);

  // self->allVarDef2 = HeapArray<VarDef>::preAllocated(32);

  self->allVarDef = PointerHeapArray__preAllocate(32);
  self->allFuncCalls = PointerHeapArray__preAllocate(32);
  self->allComptimeCalls = PointerHeapArray__preAllocate(32);
  self->allVarRefs = PointerHeapArray__preAllocate(32);
  self->allStructDef = PointerHeapArray__preAllocate(32);
  if (!self->allChildrenScopes ||
      !self->allVarDef ||
      !self->allFuncCalls ||
      !self->allComptimeCalls ||
      !self->allVarRefs ||
      !self->allStructDef)
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
  // if ((*self)->funcName)
  // {
  free((*self)->funcName);
  // }
  PointerHeapArray__free(&(*self)->allStructDef);

  // HeapArray<VarDef>::free(&(*self)->allVarDef2);

  PointerHeapArray__free(&(*self)->allVarDef);
  PointerHeapArray__free(&(*self)->allFuncCalls);
  PointerHeapArray__free(&(*self)->allComptimeCalls);
  PointerHeapArray__free(&(*self)->allVarRefs);
  PointerHeapArray__free(&(*self)->allChildrenScopes);
  free(*self);
  *self = NULL;
}
