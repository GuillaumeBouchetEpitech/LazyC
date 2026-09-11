
#include "./SourceIndexer.h"

// #include "./internals/VarDef.h"
#include "./internals/IdentifiedRef.h"
#include "./internals/ComptimeCallRef.h"
#include "./internals/SourceScope.h"

#include "stdlib/collections/HashMap.h"
#include "stdlib/collections/HashSet.h"
#include "stdlib/collections/PointerHeapArray.h"
#include "stdlib/filesystem/StreamWriter.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//
//
//

//
//
//

// MARK: SourceIndexer
typedef struct ImportedFiles
{
  // set of path (absolute path or relative path made absolute)
  HashSet *filepathsSet;
  // set of path (not absolute nor relative path, very likely dependant on includepath)
  HashSet *rawFilepathsSet;
} ImportedFiles;

typedef struct ComptimeFeatures
{
  // ex: "Vec3", etc.
  HashSet *typesToResolve;
  // ex: "HeapArena<int>" -> "HeapArena__int"
  HashMap *textsToReplace;
} ComptimeFeatures;

typedef struct SourceIndexer
{
  ImportedFiles importedFiles;

  ComptimeFeatures comptimeFeatures;

  // PointerHeapArray<SourceScope>
  PointerHeapArray *allScopes;

  SourceScope *rootScope;

  // PointerHeapArray<VarDef>
  PointerHeapArray *allVarDef;
  // PointerHeapArray<IdentifiedRef>
  PointerHeapArray *allFuncCalls;
  // PointerHeapArray<ComptimeCallRef>
  PointerHeapArray *allComptimeCalls;
  // PointerHeapArray<IdentifiedRef>
  PointerHeapArray *allVarRefs;

  HashSet *allRootDefs;
  HashSet *allDefs;
  HashSet *allRefs;
  HashSet *allComptimeFuncDefSet;

  HashSet *allExportedDefsSet;

} SourceIndexer;

// MARK: create
SourceIndexer *SourceIndexer__create(NodePos inStartPos, NodePos inEndPos)
{
  SourceIndexer *newIndexer = calloc(1, sizeof(SourceIndexer));
  if (!newIndexer)
  {
    return NULL;
  }

  newIndexer->importedFiles.filepathsSet = HashSet__preAllocate(32);
  newIndexer->importedFiles.rawFilepathsSet = HashSet__preAllocate(32);

  newIndexer->comptimeFeatures.typesToResolve = HashSet__preAllocate(32);
  newIndexer->comptimeFeatures.textsToReplace = HashMap__preAllocate(32);

  newIndexer->allScopes = PointerHeapArray__preAllocate(32);
  newIndexer->rootScope = SourceScope__create(inStartPos, inEndPos);
  newIndexer->allVarDef = PointerHeapArray__preAllocate(32);
  newIndexer->allFuncCalls = PointerHeapArray__preAllocate(32);
  newIndexer->allComptimeCalls = PointerHeapArray__preAllocate(32);
  newIndexer->allVarRefs = PointerHeapArray__preAllocate(32);
  newIndexer->allRootDefs = HashSet__preAllocate(32);
  newIndexer->allDefs = HashSet__preAllocate(32);
  newIndexer->allRefs = HashSet__preAllocate(32);
  newIndexer->allComptimeFuncDefSet = HashSet__preAllocate(32);
  newIndexer->allExportedDefsSet = HashSet__preAllocate(32);

  if (!newIndexer->importedFiles.filepathsSet ||
      !newIndexer->importedFiles.rawFilepathsSet ||
      !newIndexer->allScopes ||
      !newIndexer->rootScope ||
      !newIndexer->allVarDef ||
      !newIndexer->allFuncCalls ||
      !newIndexer->allComptimeCalls ||
      !newIndexer->allVarRefs ||
      !newIndexer->allRootDefs ||
      !newIndexer->allDefs ||
      !newIndexer->allRefs ||
      !newIndexer->allComptimeFuncDefSet ||
      !newIndexer->allExportedDefsSet)
  {
    SourceIndexer__free(&newIndexer);
    return NULL;
  }

  newIndexer->rootScope->scopeType = ROOT_SCOPE;
  PointerHeapArray__pushBack(newIndexer->allScopes, newIndexer->rootScope);

  return newIndexer;
}

// MARK: free
void SourceIndexer__free(SourceIndexer **self)
{
  if (!self || !*self)
  {
    return;
  }
  if ((*self)->importedFiles.filepathsSet)
  {
    HashSet__free(&(*self)->importedFiles.filepathsSet);
  }
  if ((*self)->importedFiles.rawFilepathsSet)
  {
    HashSet__free(&(*self)->importedFiles.rawFilepathsSet);
  }
  if ((*self)->comptimeFeatures.typesToResolve)
  {
    HashSet__free(&(*self)->comptimeFeatures.typesToResolve);
  }
  if ((*self)->comptimeFeatures.textsToReplace)
  {
    unsigned int totalKeys = 0;
    char** allKeys = HashMap__get_allKeys((*self)->comptimeFeatures.textsToReplace, &totalKeys);
    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      char* tmpValue = HashMap__get((*self)->comptimeFeatures.textsToReplace, allKeys[ii]);
      free(tmpValue);
    }
    free(allKeys);

    HashMap__free(&(*self)->comptimeFeatures.textsToReplace);
  }
  if ((*self)->allScopes)
  {
    for (unsigned int ii = 0; ii < (*self)->allScopes->len; ++ii)
    {
      SourceScope *currScope = (*self)->allScopes->data[ii];

      for (unsigned int ii = 0; ii < currScope->allVarDef->len; ++ii)
      {
        VarDef *currVar = currScope->allVarDef->data[ii];
        VarDef__free(&currVar);
      }
      for (unsigned int ii = 0; ii < currScope->allFuncCalls->len; ++ii)
      {
        IdentifiedRef *currVar = currScope->allFuncCalls->data[ii];
        IdentifiedRef__free(&currVar);
      }
      for (unsigned int ii = 0; ii < currScope->allComptimeCalls->len; ++ii)
      {
        ComptimeCallRef *currVar = currScope->allComptimeCalls->data[ii];
        ComptimeCallRef__free(&currVar);
      }
      for (unsigned int ii = 0; ii < currScope->allVarRefs->len; ++ii)
      {
        IdentifiedRef *currVar = currScope->allVarRefs->data[ii];
        IdentifiedRef__free(&currVar);
      }

      SourceScope__free(&currScope);
    }
    PointerHeapArray__free(&(*self)->allScopes);
  }
  if ((*self)->allVarDef)
  {
    PointerHeapArray__free(&(*self)->allVarDef);
  }
  if ((*self)->allFuncCalls)
  {
    PointerHeapArray__free(&(*self)->allFuncCalls);
  }
  if ((*self)->allComptimeCalls)
  {
    PointerHeapArray__free(&(*self)->allComptimeCalls);
  }
  if ((*self)->allVarRefs)
  {
    PointerHeapArray__free(&(*self)->allVarRefs);
  }

  if ((*self)->allRootDefs)
  {
    HashSet__free(&(*self)->allRootDefs);
  }
  if ((*self)->allDefs)
  {
    HashSet__free(&(*self)->allDefs);
  }
  if ((*self)->allRefs)
  {
    HashSet__free(&(*self)->allRefs);
  }
  if ((*self)->allComptimeFuncDefSet)
  {
    HashSet__free(&(*self)->allComptimeFuncDefSet);
  }
  if ((*self)->allExportedDefsSet)
  {
    HashSet__free(&(*self)->allExportedDefsSet);
  }

  free(*self);
  *self = NULL;
}

// MARK: imports
int SourceIndexer__addImport(SourceIndexer *self, const char *inImportStr)
{
  return HashSet__set(self->importedFiles.filepathsSet, inImportStr);
}

int SourceIndexer__hasImport(const SourceIndexer *self, const char *inImportStr)
{
  return HashSet__contains(self->importedFiles.filepathsSet, inImportStr);
}


char **SourceIndexer__getAllImports(const SourceIndexer *self, unsigned int *outTotalImports)
{
  return HashSet__get_allKeys(self->importedFiles.filepathsSet, outTotalImports);
}

unsigned int SourceIndexer__getTotalImports(const SourceIndexer *self)
{
  return HashSet__get_totalItems(self->importedFiles.filepathsSet);
}


int SourceIndexer__addRawImport(SourceIndexer *self, const char *inImportStr)
{
  return HashSet__set(self->importedFiles.rawFilepathsSet, inImportStr);
}

char **SourceIndexer__getAllRawImports(const SourceIndexer *self, unsigned int *outTotalImports)
{
  return HashSet__get_allKeys(self->importedFiles.rawFilepathsSet, outTotalImports);
}



// MARK: comptimeFuncs
int SourceIndexer__addComptimeFunc(SourceIndexer *self, const char *inFuncName)
{
  return HashSet__set(self->allComptimeFuncDefSet, inFuncName);
}

char **SourceIndexer__getAllComptimeFuncs(const SourceIndexer *self, unsigned int *outTotalImports)
{
  return HashSet__get_allKeys(self->allComptimeFuncDefSet, outTotalImports);
}

unsigned int SourceIndexer__getComptimeFuncs(const SourceIndexer *self)
{
  return HashSet__get_totalItems(self->allComptimeFuncDefSet);
}



// MARK: addFuncScope
int SourceIndexer__addFuncScope(SourceIndexer *self, const char *inFuncName, NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *newScope = SourceScope__create(inStartPos, inEndPos);
  if (
      !newScope ||
      PointerHeapArray__pushBack(self->allScopes, newScope) != 0 ||
      HashSet__set(self->allDefs, inFuncName) != 0 ||
      HashSet__set(self->allRootDefs, inFuncName) != 0)
  {
    SourceScope__free(&newScope);
    return -1;
  }
  newScope->scopeType = FUNC_SCOPE;
  newScope->funcName = strdup(inFuncName);
  return 0;
}

// MARK: addBlockScope
int SourceIndexer__addBlockScope(SourceIndexer *self, NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *newScope = SourceScope__create(inStartPos, inEndPos);
  if (!newScope ||
      PointerHeapArray__pushBack(self->allScopes, newScope) != 0)
  {
    SourceScope__free(&newScope);
    return -1;
  }
  newScope->scopeType = BLOCK_SCOPE;
  return 0;
}

// MARK: computeScopesHierarchy
void SourceIndexer__computeScopesHierarchy(SourceIndexer *self)
{
  // start at "index 1" -> since "index 0" is always used by the "root scope"
  for (unsigned int ii = 1; ii < self->allScopes->len; ++ii)
  {
    SourceScope *currScope = self->allScopes->data[ii];

    for (unsigned int jj = 0; jj < self->allScopes->len; ++jj)
    {
      if (ii == jj)
      {
        continue;
      }

      SourceScope *maybeParentScope = self->allScopes->data[jj];

      if (// parent contains current scope?
          maybeParentScope->startPos.index <= currScope->startPos.index &&
          currScope->endPos.index <= maybeParentScope->endPos.index)
      {
        if (// no parent -> set it
            !currScope->parentScope ||
            // has parent -> set it if "closer" to the child
            currScope->parentScope->startPos.index < maybeParentScope->startPos.index)
        {
          currScope->parentScope = maybeParentScope;
        }
      }
    }
  }

  for (unsigned int ii = 1; ii < self->allScopes->len; ++ii)
  {
    SourceScope *currScope = self->allScopes->data[ii];
    if (currScope->parentScope)
    {
      // push scope to parent
      PointerHeapArray__pushBack(currScope->parentScope->allChildrenScopes, currScope);
    }
  }
}

// MARK: findScope
static SourceScope *_SourceIndexer__findScope(const SourceIndexer *self, const NodePos *inPos, SourceScope *inCurrScope)
{
  if (inPos->index < inCurrScope->startPos.index ||
      inPos->index > inCurrScope->endPos.index)
  {
    // not contained
    return NULL;
  }

  SourceScope *finalResult = inCurrScope;

  for (unsigned int ii = 0; ii < inCurrScope->allChildrenScopes->len; ++ii)
  {

    SourceScope *childScope = inCurrScope->allChildrenScopes->data[ii];
    SourceScope *tmpResult = _SourceIndexer__findScope(self, inPos, childScope);
    if (tmpResult)
    {
      finalResult = tmpResult;
      break;
    }
  }

  return finalResult;
}

// MARK: addVarDecl
int SourceIndexer__addVarDecl(SourceIndexer *self, const char *inVarName, const char *inVarType, int inPtrLvl, NodePos inStartPos, NodePos inEndPos)
{
  VarDef *newVarDef = VarDef__create(inStartPos, inEndPos, inVarName, inVarType, inPtrLvl);
  if (!newVarDef ||
      PointerHeapArray__pushBack(self->allVarDef, newVarDef) != 0)
  {
    VarDef__free(&newVarDef);
    return -1;
  }
  if (HashSet__set(self->allDefs, inVarName) != 0)
  {
    PointerHeapArray__popBack(self->allVarDef);
    VarDef__free(&newVarDef);
    return -1;
  }
  newVarDef->parentScope = _SourceIndexer__findScope(self, &inStartPos, self->rootScope);
  if (newVarDef->parentScope &&
      newVarDef->parentScope->scopeType == ROOT_SCOPE &&
      HashSet__set(self->allRootDefs, inVarName) != 0)
  {
    PointerHeapArray__popBack(self->allVarDef);
    VarDef__free(&newVarDef);
    return -1;
  }
  if (!newVarDef->parentScope ||
      PointerHeapArray__pushBack(newVarDef->parentScope->allVarDef, newVarDef) != 0)
  {
    PointerHeapArray__popBack(self->allVarDef);
    HashSet__remove(self->allRootDefs, inVarName);
    VarDef__free(&newVarDef);
    return -1;
  }
  return 0;
}

// MARK: addFunCallRef
int SourceIndexer__addFunCallRef(SourceIndexer *self, const char *inFuncCallName, NodePos inStartPos, NodePos inEndPos)
{
  IdentifiedRef *newFuncCallRef = IdentifiedRef__create(inStartPos, inEndPos, inFuncCallName);
  if (!newFuncCallRef ||
      PointerHeapArray__pushBack(self->allFuncCalls, newFuncCallRef) != 0 ||
      HashSet__set(self->allDefs, inFuncCallName) != 0)
  {
    IdentifiedRef__free(&newFuncCallRef);
    return -1;
  }
  newFuncCallRef->parentScope = _SourceIndexer__findScope(self, &inStartPos, self->rootScope);
  if (!newFuncCallRef->parentScope ||
      PointerHeapArray__pushBack(newFuncCallRef->parentScope->allFuncCalls, newFuncCallRef) != 0)
  {
    PointerHeapArray__popBack(self->allFuncCalls);
    HashSet__remove(self->allDefs, inFuncCallName);
    IdentifiedRef__free(&newFuncCallRef);
    return -1;
  }
  return 0;
}

// MARK: comptimeCall
int SourceIndexer__addComptimeCall(SourceIndexer *self, const char *inComtimeCallName, const char *inComtimeArgs, NodePos inStartPos, NodePos inEndPos)
{
  ComptimeCallRef *newComptimeCallRef = ComptimeCallRef__create(inStartPos, inEndPos, inComtimeCallName, inComtimeArgs);
  if (!newComptimeCallRef ||
      PointerHeapArray__pushBack(self->allComptimeCalls, newComptimeCallRef) != 0 ||
      HashSet__set(self->allDefs, inComtimeCallName) != 0
    )
  {
    ComptimeCallRef__free(&newComptimeCallRef);
    return -1;
  }
  newComptimeCallRef->parentScope = _SourceIndexer__findScope(self, &inStartPos, self->rootScope);
  if (!newComptimeCallRef->parentScope ||
      PointerHeapArray__pushBack(newComptimeCallRef->parentScope->allComptimeCalls, newComptimeCallRef) != 0)
  {
    PointerHeapArray__popBack(self->allComptimeCalls);
    ComptimeCallRef__free(&newComptimeCallRef);
    return -1;
  }
  return 0;
}

const PointerHeapArray* SourceIndexer__getComptimeCallsList(const SourceIndexer *self)
{
  return self->allComptimeCalls;
}

// MARK: addVarRef
int SourceIndexer__addVarRef(SourceIndexer *self, const char *inVarRefName, NodePos inStartPos, NodePos inEndPos)
{
  if (HashSet__contains(self->allDefs, inVarRefName))
  {
    return -1;
  }

  IdentifiedRef *newVarRef = IdentifiedRef__create(inStartPos, inEndPos, inVarRefName);
  if (!newVarRef ||
      PointerHeapArray__pushBack(self->allVarRefs, newVarRef) != 0 ||
      HashSet__set(self->allRefs, inVarRefName) != 0)
  {
    IdentifiedRef__free(&newVarRef);
    return -1;
  }
  newVarRef->parentScope = _SourceIndexer__findScope(self, &inStartPos, self->rootScope);
  if (!newVarRef->parentScope ||
      PointerHeapArray__pushBack(newVarRef->parentScope->allVarRefs, newVarRef) != 0)
  {
    IdentifiedRef__free(&newVarRef);
    return -1;
  }
  return 0;
}

//MARK: exportedDef
int SourceIndexer__addExportedDef(SourceIndexer *self, const char *inVarRefName)
{
  // if (HashSet__contains(self->allExportedDefsSet, inVarRefName))
  // {
  //   return -1;
  // }
  if (HashSet__set(self->allExportedDefsSet, inVarRefName) != 0)
  {
    return -1;
  }
  return 0;
}

const HashSet* SourceIndexer__getExportedDef(const SourceIndexer *self)
{
  return self->allExportedDefsSet;
}

//MARK: ComptimeTypeToResolve
int SourceIndexer__addComptimeTypeToResolve(SourceIndexer *self, const char *inType)
{
  return HashSet__set(self->comptimeFeatures.typesToResolve, inType);
}

const HashSet* SourceIndexer__getComptimeTypesToResolve(const SourceIndexer *self)
{
  return self->comptimeFeatures.typesToResolve;
}

//MARK: ComptimeTextToReplace
int SourceIndexer__addComptimeTextToReplace(SourceIndexer *self, const char *inSrc, const char *inDst)
{
  // TODO: check if already present and error on it

  char* copiedValueStr = strdup(inDst);
  if (
    copiedValueStr == NULL ||
    HashMap__set(self->comptimeFeatures.textsToReplace, inSrc, copiedValueStr) != 0
  ) {
    return -1;
  }
  return 0;
}

const HashMap* SourceIndexer__getComptimeTextsToReplace(const SourceIndexer *self)
{
  return self->comptimeFeatures.textsToReplace;
}







//MARK: findTypename
const VarDef* SourceIndexer__findTypename(const SourceIndexer *self, const char *inVarRefName, NodePos inStartPos)
{
  const SourceScope* parentScope = _SourceIndexer__findScope(self, &inStartPos, self->rootScope);
  if (!parentScope)
  {
    return NULL;
  }

  for (const SourceScope* cursor = parentScope; cursor; cursor = cursor->parentScope)
  {
    for (unsigned int ii = 0; ii < cursor->allVarDef->len; ++ii)
    {
      VarDef* currVarDef = cursor->allVarDef->data[ii];
      if (strcmp(currVarDef->varName, inVarRefName) == 0)
      {
        return currVarDef;
      }
    }
  }

  return NULL;
}







static void _SourceIndexer__debugPrefix(StreamWriter *inStreamWriter, int inLevel, const char* inPattern)
{
  for (int ii = 0; ii < inLevel; ++ii)
  {
    StreamWriter__write(inStreamWriter, inPattern, strlen(inPattern));
  }
}

// MARK: debugTraverseScopeTree
static void _SourceIndexer__debugTraverseScopeTree(SourceScope *currScope, StreamWriter *inStreamWriter, int inLevel)
{
  _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "|---");
  char buffer[1024];
  memset(buffer, 0, 1024);
  // snprintf(buffer, 1024, "|---");
  // for (int ii = 0; ii < inLevel; ++ii)
  // {
  //   StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  // }

  snprintf(buffer, 1024, " [L%d_C%d]->[L%d_C%d] ", currScope->startPos.row, currScope->startPos.column, currScope->endPos.row, currScope->endPos.column);
  StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

  if (currScope->scopeType == ROOT_SCOPE)
  {
    snprintf(buffer, 1024, "[ROOT_SCOPE]");
  }
  else if (currScope->scopeType == FUNC_SCOPE)
  {
    snprintf(buffer, 1024, "[FUNC_SCOPE]");
  }
  else if (currScope->scopeType == BLOCK_SCOPE)
  {
    snprintf(buffer, 1024, "[BLOCK_SCOPE]");
  }
  else
  {
    snprintf(buffer, 1024, "[????_SCOPE]");
  }
  StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

  if (currScope->funcName)
  {
    snprintf(buffer, 1024, " ---> \"%s\"", currScope->funcName);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  snprintf(buffer, 1024, "\n");
  StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

  for (unsigned int ii = 0; ii < currScope->allVarDef->len; ++ii)
  {
    VarDef *currVar = currScope->allVarDef->data[ii];

    _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "=-=-");
    // snprintf(buffer, 1024, "=-=-");
    // for (int ii = 0; ii < inLevel; ++ii)
    // {
    //   StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    // }

    snprintf(buffer, 1024, " =>");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " [L%d_C%d]->[L%d_C%d] ", currVar->startPos.row, currVar->startPos.column, currVar->endPos.row, currVar->endPos.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " -> VarDef: type=\"%s\", ptrLvl=\"%d\", name=\"%s\"", currVar->typeName, currVar->pointerLevel, currVar->varName);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  for (unsigned int ii = 0; ii < currScope->allFuncCalls->len; ++ii)
  {
    IdentifiedRef *currId = currScope->allFuncCalls->data[ii];

    _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "=-=-");
    // snprintf(buffer, 1024, "=-=-");
    // for (int ii = 0; ii < inLevel; ++ii)
    // {
    //   StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    // }
    snprintf(buffer, 1024, " =>");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " [L%d_C%d]->[L%d_C%d] ", currId->startPos.row, currId->startPos.column, currId->endPos.row, currId->endPos.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " -> FunCall: name=\"%s\"", currId->varName);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  for (unsigned int ii = 0; ii < currScope->allComptimeCalls->len; ++ii)
  {
    ComptimeCallRef *currCall = currScope->allComptimeCalls->data[ii];

    _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "=-=-");
    // snprintf(buffer, 1024, "=-=-");
    // for (int ii = 0; ii < inLevel; ++ii)
    // {
    //   StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    // }
    snprintf(buffer, 1024, " =>");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " [L%d_C%d]->[L%d_C%d] ", currCall->startPos.row, currCall->startPos.column, currCall->endPos.row, currCall->endPos.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " -> ComptimeCall: name=\"%s\", args=\"%s\"", currCall->varName, currCall->argsValue);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < currCall->argsList->len; ++ii)
    {
      const char* argStr = currCall->argsList->data[ii];

      snprintf(buffer, 1024, " { args[%d]=\"%s\" }", ii, argStr);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  for (unsigned int ii = 0; ii < currScope->allVarRefs->len; ++ii)
  {
    IdentifiedRef *currId = currScope->allVarRefs->data[ii];

    _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "=-=-");
    // snprintf(buffer, 1024, "=-=-");
    // for (int ii = 0; ii < inLevel; ++ii)
    // {
    //   StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    // }
    snprintf(buffer, 1024, " =>");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " [L%d_C%d]->[L%d_C%d] ", currId->startPos.row, currId->startPos.column, currId->endPos.row, currId->endPos.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " -> VarRef: name=\"%s\"", currId->varName);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  // StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

  for (unsigned int ii = 0; ii < currScope->allChildrenScopes->len; ++ii)
  {
    SourceScope *childScope = currScope->allChildrenScopes->data[ii];
    _SourceIndexer__debugTraverseScopeTree(childScope, inStreamWriter, inLevel + 1);
  }
}

// MARK: debugTree
void SourceIndexer__debugScopeTree(SourceIndexer *self, const char* inBaseDir, StreamWriter *inStreamWriter)
{

  {

    char buffer[1024];
    memset(buffer, 0, 1024);

    {
      unsigned int totalItems = 0;
      char** allKeys = HashSet__get_allKeys(self->importedFiles.filepathsSet, &totalItems);

      snprintf(buffer, 1024, "IMPORTS (total: %d)\n", totalItems);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

      const unsigned int toSkip = strlen(inBaseDir);

      for (unsigned int ii = 0; ii < totalItems; ++ii)
      {
        const char* currPath = allKeys[ii];
        const char* relPath = currPath + toSkip + 1;

        snprintf(buffer, 1024, "-> filepath=%s\n", relPath);
        StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
      }

      free(allKeys);
    }

    // HashMap *textsToReplace; // ex: "HeapArena<int>" -> "HeapArena__int"
    {
      unsigned int totalItems = 0;
      EntryItem* items = HashMap__get_allItems(self->comptimeFeatures.textsToReplace, &totalItems);

      snprintf(buffer, 1024, "TEXTS_TO_REPLACE (total: %d)\n", totalItems);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

      for (unsigned int ii = 0; ii < totalItems; ++ii)
      {
        const char* replacement = items[ii].value;
        snprintf(buffer, 1024, "-> %s=%s\n", items[ii].key, replacement);
        StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
      }

      free(items);
    }

    // HashSet *typesToResolve; // ex: "Vec3", etc.
    {
      unsigned int totalItems = 0;
      char** allKeys = HashSet__get_allKeys(self->comptimeFeatures.typesToResolve, &totalItems);

      snprintf(buffer, 1024, "TYPES_TO_RESOLVE (total: %d)\n", totalItems);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

      for (unsigned int ii = 0; ii < totalItems; ++ii)
      {
        snprintf(buffer, 1024, "-> type=%s\n", allKeys[ii]);
        StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
      }

      free(allKeys);
    }

  }


  {
    unsigned int totalKeys = 0;
    char** allKeys = HashSet__get_allKeys(self->allExportedDefsSet, &totalKeys);

    char buffer[1024];
    memset(buffer, 0, 1024);

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      snprintf(buffer, 1024, " -> exported=%s\n", allKeys[ii]);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    free(allKeys);
  }


  _SourceIndexer__debugTraverseScopeTree(self->rootScope, inStreamWriter, 1);
}
