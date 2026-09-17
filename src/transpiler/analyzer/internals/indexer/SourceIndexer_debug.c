
#include "./SourceIndexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// MARK: debugPrefix
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

  snprintf(buffer, 1024, " [L%02d_C%02d]->[L%02d_C%02d] ", currScope->startPos.row, currScope->startPos.column, currScope->endPos.row, currScope->endPos.column);
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
  else if (currScope->scopeType == STRUCT_SCOPE)
  {
    snprintf(buffer, 1024, "[STRUCT_SCOPE]");
  }
  else if (currScope->scopeType == ENUM_SCOPE)
  {
    snprintf(buffer, 1024, "[ENUM_SCOPE]");
  }
  else if (currScope->scopeType == UNION_SCOPE)
  {
    snprintf(buffer, 1024, "[UNION_SCOPE]");
  }
  else if (currScope->scopeType == TYPEDEF_SCOPE)
  {
    snprintf(buffer, 1024, "[TYPEDEF_SCOPE]");
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

  for (unsigned int ii = 0; ii < currScope->allVarDef.len; ++ii)
  {
    VarDef *currVar = &currScope->allVarDef.data[ii];

    _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "=-=-");

    snprintf(buffer, 1024, " =>");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " [L%02d_C%02d]->[L%02d_C%02d] ", currVar->startPos.row, currVar->startPos.column, currVar->endPos.row, currVar->endPos.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " -> VarDef: type=\"%s\", ptrLvl=\"%d\", name=\"%s\"", currVar->typeName, currVar->pointerLevel, currVar->varName);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  for (unsigned int ii = 0; ii < currScope->allFuncCalls.len; ++ii)
  {
    IdentifiedRef *currId = &currScope->allFuncCalls.data[ii];

    _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "=-=-");

    snprintf(buffer, 1024, " =>");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " [L%02d_C%02d]->[L%02d_C%02d] ", currId->startPos.row, currId->startPos.column, currId->endPos.row, currId->endPos.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " -> FunCall: name=\"%s\"", currId->varName);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  for (unsigned int ii = 0; ii < currScope->allComptimeCalls.len; ++ii)
  {
    ComptimeCallRef *currCall = &currScope->allComptimeCalls.data[ii];

    _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "=-=-");

    snprintf(buffer, 1024, " =>");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " [L%02d_C%02d]->[L%02d_C%02d] ", currCall->startPos.row, currCall->startPos.column, currCall->endPos.row, currCall->endPos.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " -> ComptimeCall: name=\"%s\", args=\"%s\"", currCall->varName, currCall->argsValue);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < currCall->argsList.len; ++ii)
    {
      const StringData* argStr = &currCall->argsList.data[ii];

      snprintf(buffer, 1024, " { args[%d]=\"%s\" }", ii, argStr->data);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  for (unsigned int ii = 0; ii < currScope->allVarRefs.len; ++ii)
  {
    IdentifiedRef *currId = &currScope->allVarRefs.data[ii];

    _SourceIndexer__debugPrefix(inStreamWriter, inLevel, "=-=-");

    snprintf(buffer, 1024, " =>");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " [L%02d_C%02d]->[L%02d_C%02d] ", currId->startPos.row, currId->startPos.column, currId->endPos.row, currId->endPos.column);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, " -> VarRef: name=\"%s\"", currId->varName);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    snprintf(buffer, 1024, "\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  // StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

  for (unsigned int ii = 0; ii < currScope->allChildrenScopes.len; ++ii)
  {
    SourceScope *childScope = currScope->allChildrenScopes.data[ii];
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

      snprintf(buffer, 1024, "\nIMPORTS (total: %d)\n", totalItems);
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

      snprintf(buffer, 1024, "\nTEXTS_TO_REPLACE (total: %d)\n", totalItems);
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

      snprintf(buffer, 1024, "\nTYPES_TO_RESOLVE (total: %d)\n", totalItems);
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

    snprintf(buffer, 1024, "\nTOTAL EXPORTED (total: %d)\n", totalKeys);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      snprintf(buffer, 1024, " -> exported=%s\n", allKeys[ii]);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    free(allKeys);
  }

  {
    unsigned int totalKeys = 0;
    char** allKeys = HashSet__get_allKeys(self->dependencies.allFunctions, &totalKeys);

    char buffer[1024];
    memset(buffer, 0, 1024);

    snprintf(buffer, 1024, "\nDEP---ALL-FUNCTIONS (total: %d)\n", totalKeys);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      snprintf(buffer, 1024, " -> dep-func=%s\n", allKeys[ii]);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    free(allKeys);
  }

  {
    unsigned int totalKeys = 0;
    char** allKeys = HashSet__get_allKeys(self->dependencies.allTypes, &totalKeys);

    char buffer[1024];
    memset(buffer, 0, 1024);

    snprintf(buffer, 1024, "\nDEP---ALL-TYPES (total: %d)\n", totalKeys);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      snprintf(buffer, 1024, " -> dep-type=%s\n", allKeys[ii]);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    free(allKeys);
  }

  {
    unsigned int totalKeys = 0;
    char** allKeys = HashSet__get_allKeys(self->dependencies.provideFunctions, &totalKeys);

    char buffer[1024];
    memset(buffer, 0, 1024);

    snprintf(buffer, 1024, "\nDEP---PROVIDED-FUNCTIONS (total: %d)\n", totalKeys);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      snprintf(buffer, 1024, " -> dep-func=%s\n", allKeys[ii]);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    free(allKeys);
  }

  {
    unsigned int totalKeys = 0;
    char** allKeys = HashSet__get_allKeys(self->dependencies.provideTypes, &totalKeys);

    char buffer[1024];
    memset(buffer, 0, 1024);

    snprintf(buffer, 1024, "\nDEP---PROVIDED-TYPES (total: %d)\n", totalKeys);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      snprintf(buffer, 1024, " -> dep-type=%s\n", allKeys[ii]);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    free(allKeys);
  }

  {
    unsigned int totalKeys = 0;
    char** allKeys = HashSet__get_allKeys(self->dependencies.requireFunctions, &totalKeys);

    char buffer[1024];
    memset(buffer, 0, 1024);

    snprintf(buffer, 1024, "\nDEP---REQUIRE-FUNCTIONS (total: %d)\n", totalKeys);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      snprintf(buffer, 1024, " -> dep-func=%s\n", allKeys[ii]);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    free(allKeys);
  }

  {
    unsigned int totalKeys = 0;
    char** allKeys = HashSet__get_allKeys(self->dependencies.requireTypes, &totalKeys);

    char buffer[1024];
    memset(buffer, 0, 1024);

    snprintf(buffer, 1024, "\nDEP---REQUIRE-TYPES (total: %d)\n", totalKeys);
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));

    for (unsigned int ii = 0; ii < totalKeys; ++ii)
    {
      snprintf(buffer, 1024, " -> dep-type=%s\n", allKeys[ii]);
      StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
    }

    free(allKeys);
  }

  {
    char buffer[1024];
    memset(buffer, 0, 1024);

    snprintf(buffer, 1024, "\nALL SCOPES\n");
    StreamWriter__write(inStreamWriter, buffer, strlen(buffer));
  }

  _SourceIndexer__debugTraverseScopeTree(self->rootScope, inStreamWriter, 1);
}


