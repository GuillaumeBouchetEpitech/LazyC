
#include "./VarDef.h"

#include <stdlib.h>
#include <string.h>

VarDef VarDef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName,
    const char *inTypeName,
    int inPointerLevel)
{
  VarDef newVarDef;
  memset(&newVarDef, 0, sizeof(VarDef));

  newVarDef.startPos = inStartPos;
  newVarDef.endPos = inEndPos;
  newVarDef.varName = strdup(inVarName);
  newVarDef.typeName = strdup(inTypeName);
  newVarDef.pointerLevel = inPointerLevel;

  return newVarDef;
}

void VarDef__free(VarDef *self)
{
  if (!self)
  {
    return;
  }
  free(self->varName);
  free(self->typeName);
  memset(self, 0, sizeof(VarDef));
}
