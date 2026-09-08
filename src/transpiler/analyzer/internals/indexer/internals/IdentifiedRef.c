
#include "./IdentifiedRef.h"

#include <stdlib.h>
#include <string.h>

IdentifiedRef *IdentifiedRef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName
) {
  IdentifiedRef *newIdentifiedRef = calloc(1, sizeof(IdentifiedRef));
  if (!newIdentifiedRef)
  {
    return NULL;
  }
  newIdentifiedRef->startPos = inStartPos;
  newIdentifiedRef->endPos = inEndPos;

  newIdentifiedRef->varName = strdup(inVarName);

  return newIdentifiedRef;
}

void IdentifiedRef__free(IdentifiedRef **self)
{
  if (!self || !*self)
  {
    return;
  }
  free((*self)->varName);
  // free((*self)->typeName);
  free(*self);
  *self = NULL;
}
