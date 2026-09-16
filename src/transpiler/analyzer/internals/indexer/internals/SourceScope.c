
#include "SourceScope.h"

#include <stdlib.h>

SourceScope *SourceScope__create(NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *self = calloc(1, sizeof(SourceScope));
  if (!self)
  {
    return NULL;
  }

  self->funcName = NULL;
  self->parentScope = NULL;
  self->allChildrenScopes = HeapArray<SourceScope*>::preAllocated(32);

  self->scopeType = UNKNOWN_SCOPE;
  self->startPos = inStartPos;
  self->endPos = inEndPos;

  self->allVarDef = HeapArray<VarDef>::preAllocated(32);
  self->allFuncCalls = HeapArray<IdentifiedRef>::preAllocated(32);
  self->allComptimeCalls = HeapArray<ComptimeCallRef>::preAllocated(32);
  self->allVarRefs = HeapArray<IdentifiedRef>::preAllocated(32);


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

  for (unsigned int ii = 0; ii < (*self)->allComptimeCalls.len; ++ii)
  {
    ComptimeCallRef *currCall = &(*self)->allComptimeCalls.data[ii];
    ComptimeCallRef__free(currCall);
  }

  for (unsigned int ii = 0; ii < (*self)->allVarRefs.len; ++ii)
  {
    IdentifiedRef *currVar = &(*self)->allVarRefs.data[ii];
    IdentifiedRef__free(currVar);
  }

  HeapArray<VarDef>::free(&(*self)->allVarDef);
  HeapArray<IdentifiedRef>::free(&(*self)->allFuncCalls);
  HeapArray<ComptimeCallRef>::free(&(*self)->allComptimeCalls);
  HeapArray<IdentifiedRef>::free(&(*self)->allVarRefs);
  HeapArray<SourceScope*>::free(&(*self)->allChildrenScopes);
  free(*self);
  *self = NULL;
}
