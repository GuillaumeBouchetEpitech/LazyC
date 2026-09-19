
#pragma once

#include "../parser/SourceParser.h"

#include "./internals/VarDef.h"
#include "./internals/IdentifiedRef.h"
#include "./internals/ComptimeCallRef.h"
#include "./internals/SourceScope.h"
#include "./internals/Dependencies.lc"

#include "stdlib/core/panic.h"
#include "stdlib/collections/HashMap.h"
#include "stdlib/collections/HashSet.h"
#include "stdlib/collections/PointerHeapArray.h"
#include "stdlib/filesystem/StreamWriter.h"

#include "stdlib/collections/HeapArray.lc"

// MARK: SourceIndexer
typedef struct ImportedFiles
{
  // set of path (absolute path or relative path made absolute)
  HashSet *filepathsSet;
  // set of path (not absolute nor relative path, very likely dependant on includepath)
  HashSet *rawFilepathsSet;
}
ImportedFiles;

typedef struct ComptimeFeatures
{
  // ex: "Vec3", etc.
  HashSet *typesToResolve;
  // ex: "HeapArena<int>" -> "HeapArena__int"
  HashMap *textsToReplace;

  HashSet *allComptimeFuncDefSet;
}
ComptimeFeatures;

//MARK: SourceIndexer
typedef struct SourceIndexer
{
  ImportedFiles importedFiles;

  ComptimeFeatures comptimeFeatures;

  HeapArray<SourceScope*> allScopes;
  SourceScope *rootScope;

  HeapArray<VarDef> allFileVarDef;
  HeapArray<IdentifiedRef> allFileFuncCalls;
  HeapArray<ComptimeCallRef> allFileComptimeCalls;
  HeapArray<IdentifiedRef> allFileVarRefs;

  // HashSet *allRootDefs;
  HashSet *allDefIdentifiers;
  // HashSet *allRefs;

  Dependencies dependencies;

  HashSet *allExportedDefsSet;
}
SourceIndexer;

SourceIndexer *SourceIndexer__create(NodePos inStartPos, NodePos inEndPos);
void SourceIndexer__free(SourceIndexer **self);

int SourceIndexer__addImport(SourceIndexer *self, const char *inImportStr);
int SourceIndexer__hasImport(const SourceIndexer *self, const char *inImportStr);
char **SourceIndexer__getAllImports(const SourceIndexer *self, unsigned int *outTotalImports);
unsigned int SourceIndexer__getTotalImports(const SourceIndexer *self);

int SourceIndexer__addRawImport(SourceIndexer *self, const char *inImportStr);
char **SourceIndexer__getAllRawImports(const SourceIndexer *self, unsigned int *outTotalImports);

int SourceIndexer__addComptimeFunc(SourceIndexer *self, const char *inFuncName);

int SourceIndexer__addFuncScope(SourceIndexer *self, const char *inFuncName, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addBlockScope(SourceIndexer *self, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addStructScope(SourceIndexer *self, const char *inTypeName, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addEnumScope(SourceIndexer *self, const char *inTypeName, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addUnionScope(SourceIndexer *self, const char *inTypeName, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addTypedefScope(SourceIndexer *self, const char *inTypeName, NodePos inStartPos, NodePos inEndPos);
void SourceIndexer__computeScopesHierarchy(SourceIndexer *self);
void SourceIndexer__computeDependencies(SourceIndexer *self);

int SourceIndexer__addVarDecl(SourceIndexer *self, const char *inVarName, const char *inVarType, int inPtrLvl, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addFunCallRef(SourceIndexer *self, const char *inFuncCallName, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addComptimeCall(SourceIndexer *self, const char *inComtimeCallName, const char *inComtimeArgs, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addVarRef(SourceIndexer *self, const char *inVarRefName, NodePos inStartPos, NodePos inEndPos);
int SourceIndexer__addExportedDef(SourceIndexer *self, const char *inVarRefName);

// HashSet<char*>
const HashSet* SourceIndexer__getExportedDef(const SourceIndexer *self);

const HeapArray<ComptimeCallRef>* SourceIndexer__getComptimeCallsList(const SourceIndexer *self);


// ex: "Vec3", etc.
int SourceIndexer__addComptimeTypeToResolve(SourceIndexer *self, const char *inType);
const HashSet* SourceIndexer__getComptimeTypesToResolve(const SourceIndexer *self);

// ex: "HeapArena<int>" -> "HeapArena__int"
int SourceIndexer__addComptimeTextToReplace(SourceIndexer *self, const char *inSrc, const char *inDst);
const HashMap* SourceIndexer__getComptimeTextsToReplace(const SourceIndexer *self);


const VarDef* SourceIndexer__findTypename(const SourceIndexer *self, const char *inVarRefName, NodePos inStartPos);


void SourceIndexer__debugScopeTree(SourceIndexer *self, const char* inBaseDir, StreamWriter *streamWriter);


int SourceIndexer__hasMainFunction(const SourceIndexer *self);

int SourceIndexer__require(const SourceIndexer *self, const SourceIndexer *other);

