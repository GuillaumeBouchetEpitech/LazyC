
#include "./IdentifiedRef.h"

#include <stdlib.h>
#include <string.h>

IdentifiedRef IdentifiedRef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName
) {
  IdentifiedRef newIdentifiedRef;
  memset(&newIdentifiedRef, 0, sizeof(IdentifiedRef));

  newIdentifiedRef.startPos = inStartPos;
  newIdentifiedRef.endPos = inEndPos;

  newIdentifiedRef.varName = strdup(inVarName);

  return newIdentifiedRef;
}

void IdentifiedRef__free(IdentifiedRef *self)
{
  if (!self)
  {
    return;
  }
  free(self->varName);
  memset(self, 0, sizeof(IdentifiedRef));
}
