
#include "./SourceIndexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//
//
//

//
//
//

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

  newIndexer->allScopes = HeapArray<SourceScope*>::preAllocated(32);
  newIndexer->rootScope = SourceScope__create(inStartPos, inEndPos);
  newIndexer->allFileVarDef = HeapArray<VarDef>::preAllocated(32);
  newIndexer->allFileFuncCalls = HeapArray<IdentifiedRef>::preAllocated(32);
  newIndexer->allFileComptimeCalls = HeapArray<ComptimeCallRef>::preAllocated(32);
  newIndexer->allFileVarRefs = HeapArray<IdentifiedRef>::preAllocated(32);
  // newIndexer->allRootDefs = HashSet__preAllocate(32);
  newIndexer->allDefIdentifiers = HashSet__preAllocate(32);
  // newIndexer->allRefs = HashSet__preAllocate(32);
  newIndexer->comptimeFeatures.allComptimeFuncDefSet = HashSet__preAllocate(32);
  newIndexer->allExportedDefsSet = HashSet__preAllocate(32);

  if (!newIndexer->importedFiles.filepathsSet ||
      !newIndexer->importedFiles.rawFilepathsSet ||
      !newIndexer->rootScope ||
      // !newIndexer->allRootDefs ||
      !newIndexer->allDefIdentifiers ||
      // !newIndexer->allRefs ||
      !newIndexer->comptimeFeatures.allComptimeFuncDefSet ||
      !newIndexer->allExportedDefsSet)
  {
    SourceIndexer__free(&newIndexer);
    return NULL;
  }

  newIndexer->dependencies = Dependencies__create();

  newIndexer->rootScope->scopeType = ROOT_SCOPE;

  HeapArray<SourceScope*>::pushBack(&newIndexer->allScopes, newIndexer->rootScope);

  return newIndexer;
}

// MARK: free
void SourceIndexer__free(SourceIndexer **self)
{
  if (!self || !*self)
  {
    return;
  }

  Dependencies__free(&(*self)->dependencies);

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

  {
    for (unsigned int ii = 0; ii < (*self)->allScopes.len; ++ii)
    {
      SourceScope *currScope = (*self)->allScopes.data[ii];
      SourceScope__free(&currScope);
    }

    HeapArray<SourceScope*>::free(&(*self)->allScopes);
  }

  HeapArray<VarDef>::free(&(*self)->allFileVarDef);
  HeapArray<IdentifiedRef>::free(&(*self)->allFileFuncCalls);
  HeapArray<ComptimeCallRef>::free(&(*self)->allFileComptimeCalls);
  HeapArray<IdentifiedRef>::free(&(*self)->allFileVarRefs);

  // if ((*self)->allRootDefs)
  // {
  //   HashSet__free(&(*self)->allRootDefs);
  // }
  if ((*self)->allDefIdentifiers)
  {
    HashSet__free(&(*self)->allDefIdentifiers);
  }
  // if ((*self)->allRefs)
  // {
  //   HashSet__free(&(*self)->allRefs);
  // }
  if ((*self)->comptimeFeatures.allComptimeFuncDefSet)
  {
    HashSet__free(&(*self)->comptimeFeatures.allComptimeFuncDefSet);
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
  return HashSet__set(self->comptimeFeatures.allComptimeFuncDefSet, inFuncName);
}

char **SourceIndexer__getAllComptimeFuncs(const SourceIndexer *self, unsigned int *outTotalImports)
{
  return HashSet__get_allKeys(self->comptimeFeatures.allComptimeFuncDefSet, outTotalImports);
}

unsigned int SourceIndexer__getComptimeFuncs(const SourceIndexer *self)
{
  return HashSet__get_totalItems(self->comptimeFeatures.allComptimeFuncDefSet);
}



// MARK: addFuncScope
int SourceIndexer__addFuncScope(SourceIndexer *self, const char *inFuncName, NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *newScope = SourceScope__create(inStartPos, inEndPos);
  if (!newScope)
  {
    SourceScope__free(&newScope);
    return -1;
  }

  HeapArray<SourceScope*>::pushBack(&self->allScopes, newScope);

  if (
    HashSet__set(self->allDefIdentifiers, inFuncName) != 0 //||
    // HashSet__set(self->allRootDefs, inFuncName) != 0
  ) {
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
  if (!newScope)
  {
    SourceScope__free(&newScope);
    return -1;
  }
  newScope->scopeType = BLOCK_SCOPE;
  HeapArray<SourceScope*>::pushBack(&self->allScopes, newScope);
  return 0;
}

// MARK: addStructScope
int SourceIndexer__addStructScope(SourceIndexer *self, const char *inTypeName, NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *newScope = SourceScope__create(inStartPos, inEndPos);
  if (!newScope)
  {
    SourceScope__free(&newScope);
    return -1;
  }

  newScope->scopeType = STRUCT_SCOPE;
  if (inTypeName) {
    newScope->funcName = strdup(inTypeName);
  }

  HeapArray<SourceScope*>::pushBack(&self->allScopes, newScope);
  return 0;
}

int SourceIndexer__addEnumScope(SourceIndexer *self, const char *inTypeName, NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *newScope = SourceScope__create(inStartPos, inEndPos);
  if (!newScope)
  {
    SourceScope__free(&newScope);
    return -1;
  }

  newScope->scopeType = ENUM_SCOPE;
  if (inTypeName) {
    newScope->funcName = strdup(inTypeName);
  }

  HeapArray<SourceScope*>::pushBack(&self->allScopes, newScope);
  return 0;
}

int SourceIndexer__addUnionScope(SourceIndexer *self, const char *inTypeName, NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *newScope = SourceScope__create(inStartPos, inEndPos);
  if (!newScope)
  {
    SourceScope__free(&newScope);
    return -1;
  }

  newScope->scopeType = UNION_SCOPE;
  if (inTypeName) {
    newScope->funcName = strdup(inTypeName);
  }

  HeapArray<SourceScope*>::pushBack(&self->allScopes, newScope);
  return 0;
}

int SourceIndexer__addTypedefScope(SourceIndexer *self, const char *inTypeName, NodePos inStartPos, NodePos inEndPos)
{
  SourceScope *newScope = SourceScope__create(inStartPos, inEndPos);
  if (!newScope)
  {
    SourceScope__free(&newScope);
    return -1;
  }

  newScope->scopeType = TYPEDEF_SCOPE;
  if (inTypeName) {
    newScope->funcName = strdup(inTypeName);
  }

  HeapArray<SourceScope*>::pushBack(&self->allScopes, newScope);
  return 0;
}








// MARK: findScope
static SourceScope *_SourceIndexer__findScopeFromStartEnd(const SourceIndexer *self, const NodePos *inStartPos, const NodePos *inEndPos, SourceScope *inCurrScope)
{
  if (!(
    inCurrScope->startPos.index <= inStartPos->index &&
    inEndPos->index <= inCurrScope->endPos.index
  )) {
    // not contained
    return NULL;
  }

  for (unsigned int ii = 0; ii < inCurrScope->allChildrenScopes.len; ++ii)
  {
    SourceScope *childScope = inCurrScope->allChildrenScopes.data[ii];
    SourceScope *tmpResult = _SourceIndexer__findScopeFromStartEnd(self, inStartPos, inEndPos, childScope);
    if (tmpResult)
    {
      return tmpResult;
    }
  }

  return inCurrScope;
}

// MARK: findScope
static SourceScope *_SourceIndexer__findScope(const SourceIndexer *self, const NodePos *inPos, SourceScope *inCurrScope)
{
  return _SourceIndexer__findScopeFromStartEnd(self, inPos, inPos, inCurrScope);
}



// MARK: computeScopesHierarchy
void SourceIndexer__computeScopesHierarchy(SourceIndexer *self)
{
  // start at "index 1" -> since "index 0" is always used by the "root scope"
  for (unsigned int ii = 1; ii < self->allScopes.len; ++ii)
  {
    SourceScope *currScope = self->allScopes.data[ii];

    for (unsigned int jj = 0; jj < self->allScopes.len; ++jj)
    {
      if (ii == jj)
      {
        continue;
      }

      SourceScope *maybeParentScope = self->allScopes.data[jj];

      if (// parent contains current scope?
          maybeParentScope->startPos.index <= currScope->startPos.index &&
          currScope->endPos.index <= maybeParentScope->endPos.index)
      {
        if (// no parent -> set it
          !currScope->parentScope ||
          // has parent -> set it if "closer" to the child
          (
            currScope->parentScope->startPos.index < maybeParentScope->startPos.index // &&
            // currScope->endPos.index <= maybeParentScope->endPos.index
          )
        ) {
          currScope->parentScope = maybeParentScope;
        }
      }
    }


    // if (currScope->scopeType == ROOT_SCOPE)
    // {
    //   printf(" -==-> [ROOT_SCOPE]\n");
    // }
    // else if (currScope->scopeType == FUNC_SCOPE)
    // {
    //   printf(" -==-> [FUNC_SCOPE]\n");
    // }
    // else if (currScope->scopeType == BLOCK_SCOPE)
    // {
    //   printf(" -==-> [BLOCK_SCOPE]\n");
    // }
    // else if (currScope->scopeType == STRUCT_SCOPE)
    // {
    //   printf(" -==-> [STRUCT_SCOPE]\n");
    // }
    // else if (currScope->scopeType == ENUM_SCOPE)
    // {
    //   printf(" -==-> [ENUM_SCOPE]\n");
    // }
    // else if (currScope->scopeType == UNION_SCOPE)
    // {
    //   printf(" -==-> [UNION_SCOPE]\n");
    // }
    // else if (currScope->scopeType == TYPEDEF_SCOPE)
    // {
    //   printf(" -==-> [TYPEDEF_SCOPE]\n");
    // }
    // else
    // {
    //   printf(" -==-> [????_SCOPE]\n");
    // }


    if (currScope->parentScope == NULL)
    {
      panic("no parent scope found for a child scope");
    }

  }

  for (unsigned int ii = 1; ii < self->allScopes.len; ++ii)
  {
    SourceScope *currScope = self->allScopes.data[ii];
    if (currScope->parentScope)
    {
      // push this scope as a "child scope" to their newly found "parent scope"
      HeapArray<SourceScope*>::pushBack(&currScope->parentScope->allChildrenScopes, currScope);
    }
  }

  {
    for (unsigned int ii = 0; ii < self->allFileVarDef.len; ++ii)
    {
      VarDef* currVarDef = &self->allFileVarDef.data[ii];
      // currVarDef->parentScope = _SourceIndexer__findScope(self, &currVarDef->startPos, self->rootScope);
      currVarDef->parentScope = _SourceIndexer__findScopeFromStartEnd(self, &currVarDef->startPos, &currVarDef->endPos, self->rootScope);

      // printf(" -{VARDEF[%d]}-> %s (%s)\n", ii, currVarDef->varName, currVarDef->typeName);

      HeapArray<VarDef>::pushBackRef(&currVarDef->parentScope->allVarDef, currVarDef);
    }

    for (unsigned int ii = 0; ii < self->allFileFuncCalls.len; ++ii)
    {
      IdentifiedRef* currFuncCall = &self->allFileFuncCalls.data[ii];
      currFuncCall->parentScope = _SourceIndexer__findScope(self, &currFuncCall->startPos, self->rootScope);
      if (!currFuncCall->parentScope) {
        panic("count not find a parent scope for a function");
      }
      HeapArray<IdentifiedRef>::pushBackRef(&currFuncCall->parentScope->allFuncCalls, currFuncCall);
    }

    for (unsigned int ii = 0; ii < self->allFileComptimeCalls.len; ++ii)
    {
      ComptimeCallRef* currComptimeCall = &self->allFileComptimeCalls.data[ii];
      currComptimeCall->parentScope = _SourceIndexer__findScope(self, &currComptimeCall->startPos, self->rootScope);
      HeapArray<ComptimeCallRef>::pushBackRef(&currComptimeCall->parentScope->allComptimeCalls, currComptimeCall);
    }

    for (unsigned int ii = 0; ii < self->allFileVarRefs.len; ++ii)
    {
      IdentifiedRef* currVarRef = &self->allFileVarRefs.data[ii];
      currVarRef->parentScope = _SourceIndexer__findScope(self, &currVarRef->startPos, self->rootScope);
      HeapArray<IdentifiedRef>::pushBackRef(&currVarRef->parentScope->allVarRefs, currVarRef);
    }

  }
}





// MARK: computeDependencies
void SourceIndexer__computeDependencies(SourceIndexer *self)
{

  for (unsigned int ii = 0; ii < self->rootScope->allChildrenScopes.len; ++ii)
  {
    const SourceScope *childScope = self->rootScope->allChildrenScopes.data[ii];
    const char* funcName = childScope->funcName;

    if (funcName == NULL) {
      continue;
    }

    const int isExported = HashSet__contains(self->allExportedDefsSet, funcName);

    switch (childScope->scopeType)
    {
    case FUNC_SCOPE:
      HashSet__set(self->dependencies.allFunctions, funcName);
      if (isExported != 0) {
        HashSet__set(self->dependencies.provideFunctions, funcName);
      }
      break;
    case STRUCT_SCOPE:
    case ENUM_SCOPE:
    case UNION_SCOPE:
    case TYPEDEF_SCOPE:
      HashSet__set(self->dependencies.allTypes, funcName);
      if (isExported != 0) {
        HashSet__set(self->dependencies.provideTypes, funcName);
      }
      break;
    default:
      break;
    }
  }

  // foreign functions
  for (unsigned int ii = 0; ii < self->allFileFuncCalls.len; ++ii)
  {
    IdentifiedRef* currVarDef = &self->allFileFuncCalls.data[ii];
    if (HashSet__contains(self->dependencies.allFunctions, currVarDef->varName) == 0) {
      HashSet__set(self->dependencies.requireFunctions, currVarDef->varName);
    }
  }

  // foreign types
  for (unsigned int ii = 0; ii < self->allFileVarDef.len; ++ii)
  {
    VarDef* currVarDef = &self->allFileVarDef.data[ii];
    if (HashSet__contains(self->dependencies.allTypes, currVarDef->typeName) == 0) {
      HashSet__set(self->dependencies.requireTypes, currVarDef->typeName);
    }
  }

}





// MARK: addVarDecl
int SourceIndexer__addVarDecl(SourceIndexer *self, const char *inVarName, const char *inVarType, int inPtrLvl, NodePos inStartPos, NodePos inEndPos)
{
  VarDef newVarDef = VarDef__create(inStartPos, inEndPos, inVarName, inVarType, inPtrLvl);
  HeapArray<VarDef>::pushBackRef(&self->allFileVarDef, &newVarDef);

  if (HashSet__set(self->allDefIdentifiers, inVarName) != 0)
  {
    HeapArray<VarDef>::popBack(&self->allFileVarDef);
    VarDef__free(&newVarDef);
    return -1;
  }
  return 0;
}

// MARK: addFunCallRef
int SourceIndexer__addFunCallRef(SourceIndexer *self, const char *inFuncCallName, NodePos inStartPos, NodePos inEndPos)
{
  IdentifiedRef newFuncCallRef = IdentifiedRef::create(inStartPos, inEndPos, inFuncCallName);
  HeapArray<IdentifiedRef>::pushBackRef(&self->allFileFuncCalls, &newFuncCallRef);

  if (HashSet__set(self->allDefIdentifiers, inFuncCallName) != 0)
  {
    HeapArray<IdentifiedRef>::popBack(&self->allFileFuncCalls);
    IdentifiedRef::free(&newFuncCallRef);
    return -1;
  }
  return 0;
}

// MARK: comptimeCall
int SourceIndexer__addComptimeCall(SourceIndexer *self, const char *inComtimeCallName, const char *inComtimeArgs, NodePos inStartPos, NodePos inEndPos)
{
  ComptimeCallRef newComptimeCallRef = ComptimeCallRef__create(inStartPos, inEndPos, inComtimeCallName, inComtimeArgs);
  HeapArray<ComptimeCallRef>::pushBackRef(&self->allFileComptimeCalls, &newComptimeCallRef);
  if (HashSet__set(self->allDefIdentifiers, inComtimeCallName) != 0)
  {
    HeapArray<ComptimeCallRef>::popBack(&self->allFileComptimeCalls);
    ComptimeCallRef__free(&newComptimeCallRef);
    return -1;
  }
  return 0;
}

const HeapArray<ComptimeCallRef>* SourceIndexer__getComptimeCallsList(const SourceIndexer *self)
{
  return &self->allFileComptimeCalls;
}




// MARK: addVarRef
int SourceIndexer__addVarRef(SourceIndexer *self, const char *inVarRefName, NodePos inStartPos, NodePos inEndPos)
{
  if (HashSet__contains(self->allDefIdentifiers, inVarRefName))
  {
    return -1;
  }

  IdentifiedRef newVarRef = IdentifiedRef__create(inStartPos, inEndPos, inVarRefName);
  HeapArray<IdentifiedRef>::pushBackRef(&self->allFileVarRefs, &newVarRef);

  // if (HashSet__set(self->allRefs, inVarRefName) != 0)
  // {
  //   IdentifiedRef__free(&newVarRef);
  //   return -1;
  // }
  return 0;
}





//MARK: exportedDef
int SourceIndexer__addExportedDef(SourceIndexer *self, const char *inVarRefName)
{
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

  // direction: childScope --> parentScope
  for (const SourceScope* currScope = parentScope; currScope; currScope = currScope->parentScope)
  {
    for (unsigned int ii = 0; ii < currScope->allVarDef.len; ++ii)
    {
      VarDef* currVarDef = &currScope->allVarDef.data[ii];
      if (strcmp(currVarDef->varName, inVarRefName) == 0)
      {
        return currVarDef;
      }
    }
  }

  return NULL;
}





int SourceIndexer__hasMainFunction(const SourceIndexer *self)
{
  for (unsigned int ii = 0; ii < self->rootScope->allChildrenScopes.len; ++ii)
  {
    const SourceScope *childScope = self->rootScope->allChildrenScopes.data[ii];
    if (
      childScope->scopeType == FUNC_SCOPE &&
      childScope->funcName != NULL &&
      strcmp(childScope->funcName, "main") == 0
    ) {
      return 1;
    }
  }
  return 0;
}

int SourceIndexer__require(const SourceIndexer *self, const SourceIndexer *other)
{
  return Dependencies__require(&self->dependencies, &other->dependencies);
}

