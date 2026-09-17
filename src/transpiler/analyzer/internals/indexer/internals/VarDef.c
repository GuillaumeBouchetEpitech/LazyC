
#include "./VarDef.h"

#include "stdlib/strings/trimStr.h"

#include <stdlib.h>
#include <string.h>

const char* structPattern = "struct ";
const char* enumPattern = "enum ";
  // union



VarDef VarDef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName,
    const char *inTypeName,
    int inPointerLevel)
{
  VarDef newVarDef;
  memset(&newVarDef, 0, sizeof(VarDef));

  const unsigned int structPatternLen = strlen(structPattern);
  const unsigned int enumPatternLen = strlen(enumPattern);

  char* typeCpy = strdup(inTypeName);
  char* typeStr = typeCpy;
  if (strncmp(structPattern, inTypeName, structPatternLen) == 0)
  {
    typeStr = typeStr + structPatternLen;
    typeStr = String__ltrim(typeStr);
  }
  else if (strncmp(enumPattern, inTypeName, enumPatternLen) == 0)
  {
    typeStr = typeStr + enumPatternLen;
    typeStr = String__ltrim(typeStr);
  }

  newVarDef.startPos = inStartPos;
  newVarDef.endPos = inEndPos;
  newVarDef.varName = strdup(inVarName);
  newVarDef.typeName = strdup(typeStr);
  newVarDef.pointerLevel = inPointerLevel;

  free(typeCpy);

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
