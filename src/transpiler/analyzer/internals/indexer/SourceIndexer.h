
#pragma once

#include "../parser/SourceParser.h"
#include "./internals/VarDef.h"

// opaque declaration
typedef struct SourceIndexer SourceIndexer;
// typedef struct VarDef VarDef;

// forward declaration
typedef struct PointerHeapArray PointerHeapArray;
typedef struct HashSet HashSet;
typedef struct HashMap HashMap;

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

