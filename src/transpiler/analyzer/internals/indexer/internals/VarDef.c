
#include "./VarDef.h"

#include <stdlib.h>
#include <string.h>

VarDef *VarDef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName,
    const char *inTypeName,
    int inPointerLevel)
{
  VarDef *newVarDef = calloc(1, sizeof(VarDef));
  if (!newVarDef)
  {
    return NULL;
  }
  newVarDef->startPos = inStartPos;
  newVarDef->endPos = inEndPos;
  newVarDef->varName = strdup(inVarName);
  newVarDef->typeName = strdup(inTypeName);
  newVarDef->pointerLevel = inPointerLevel;

  return newVarDef;
}

void VarDef__free(VarDef **self)
{
  if (!self || !*self)
  {
    return;
  }
  free((*self)->varName);
  free((*self)->typeName);
  free(*self);
  *self = NULL;
}
