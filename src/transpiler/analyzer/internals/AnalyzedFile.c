
#include "./AnalyzedFile.h"


// #include "./SourceAnalyzer.h"

#include "./parser/SourceParser.h"
#include "stdlib/collections/HashMap.h"
// #include "stdlib/collections/HashSet.h"
// #include "stdlib/collections/PointerHeapArray.h"
// #include "stdlib/filesystem/readFile.h"
#include "stdlib/filesystem/pathUtils.h"
#include "stdlib/strings/trimStr.h"

// #include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>




// forward declaration(s)
int AnalyzedFile__queryImports(AnalyzedFile *self);
int AnalyzedFile__queryScopes(AnalyzedFile *self);
int AnalyzedFile__queryStructDef(AnalyzedFile *self);
int AnalyzedFile__queryFuncSignatures(AnalyzedFile *self);
int AnalyzedFile__queryVarDef(AnalyzedFile *self);
int AnalyzedFile__queryFuncCalls(AnalyzedFile *self);
int AnalyzedFile__queryComptimeCalls(AnalyzedFile *self);
int AnalyzedFile__queryVarRef(AnalyzedFile *self);
int AnalyzedFile__queryComptimeComments(AnalyzedFile *self);

//MARK: AnalyzedFile_create
AnalyzedFile *AnalyzedFile__create(SourceParser *inParser, const char *inFilepath)
{
  AnalyzedFile *analyzedFile = calloc(1, sizeof(AnalyzedFile));
  if (!analyzedFile)
  {
    return NULL;
  }
  analyzedFile->filepath = strdup(inFilepath);
  analyzedFile->parsedFile = SourceParser__parse(inParser, inFilepath);

  if (!analyzedFile->filepath ||
      !analyzedFile->parsedFile)
  {
    AnalyzedFile__free(&analyzedFile);
    return NULL;
  }

  {
    char* extname = Path__extname(inFilepath);

    if (strcmp(extname, ".h") == 0) { analyzedFile->fileType = SOURCE_H; }
    else if (strcmp(extname, ".c") == 0) { analyzedFile->fileType = SOURCE_C; }
    else if (strcmp(extname, ".lc") == 0) { analyzedFile->fileType = SOURCE_LC; }

    free(extname);
  }

  NodePos startPos = SourceParsedFile__getStartPos(analyzedFile->parsedFile);
  NodePos endPos = SourceParsedFile__getEndPos(analyzedFile->parsedFile);

  analyzedFile->indexer = SourceIndexer__create(startPos, endPos);

  if (!analyzedFile->indexer)
  {
    AnalyzedFile__free(&analyzedFile);
    return NULL;
  }

  if (AnalyzedFile__queryImports(analyzedFile) != 0 ||
      AnalyzedFile__queryScopes(analyzedFile) != 0 ||
      AnalyzedFile__queryStructDef(analyzedFile) != 0 ||
      AnalyzedFile__queryFuncSignatures(analyzedFile) != 0 ||
      AnalyzedFile__queryComptimeCalls(analyzedFile) != 0 ||
      AnalyzedFile__queryVarDef(analyzedFile) != 0 ||
      AnalyzedFile__queryFuncCalls(analyzedFile) != 0 ||
      AnalyzedFile__queryVarRef(analyzedFile) != 0 ||
      AnalyzedFile__queryComptimeComments(analyzedFile) != 0) {
    AnalyzedFile__free(&analyzedFile);
    return NULL;
  }

  return analyzedFile;
}

//MARK: free
void AnalyzedFile__free(AnalyzedFile **self)
{
  if (!self || !*self)
  {
    return;
  }
  if ((*self)->filepath)
  {
    free((*self)->filepath);
  }
  if ((*self)->parsedFile)
  {
    SourceParsedFile__free(&(*self)->parsedFile);
  }
  if ((*self)->indexer)
  {
    SourceIndexer__free(&(*self)->indexer);
  }
  free(*self);
  *self = NULL;
}

//MARK: queryImports
int AnalyzedFile__queryImports(AnalyzedFile *self)
{
  const char* k_queryStr = "\n"
    "\n"
    "; includes\n"
    "\n"
    "(preproc_include path: (_) @import.source) @import\n"
    "\n";

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));

  char* currDir = Path__dirname(self->filepath);

  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    if (
      HashMap__contains(cursor->captureMap, "import") &&
      HashMap__contains(cursor->captureMap, "import.source")
    ) {
      NodeData* sourceNode = HashMap__get(cursor->captureMap, "import.source");

      const char* srcStr = parsedFileContent + sourceNode->startPos.index;
      const unsigned int contentSize = sourceNode->endPos.index - sourceNode->startPos.index;

      if (
        contentSize > 2 &&
        srcStr[0] == '\"' &&
        srcStr[contentSize - 1] == '\"'
      ) {
        // #include "some-filepath"

        char* tmpContent = strndup(srcStr + 1, contentSize - 2);

        // printf(" ---> [REL] {%s}\n", tmpContent);

        if (tmpContent[0] == '/')
        {
          // path is absolute -> just add as is
          SourceIndexer__addImport(self->indexer, tmpContent);
        }
        else if (tmpContent[0] == '.')
        {
          // path is relative -> need resolving
          // TODO: must support user provided "include-path"
          char* absolutePath = Path__join(2, currDir, tmpContent);
          // printf("   -> [ABS] {%s}\n", absolutePath);

          SourceIndexer__addImport(self->indexer, absolutePath);

          free(absolutePath);
        }
        else {
          SourceIndexer__addRawImport(self->indexer, tmpContent);
        }


        free(tmpContent);
      }
      // else
      // {
      //   // #include <some-filepath>

      //   char* tmpContent = strndup(srcStr, contentSize);
      //   printf(" ---> [%s]\n", tmpContent);
      //   free(tmpContent);
      // }
    }
  }

  free(currDir);

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: queryScopes
int AnalyzedFile__queryScopes(AnalyzedFile *self)
{
  const char* k_queryStr = "\n"
    "\n"
    "; block scopes\n"
    "\n"
    "(compound_statement) @block.scope\n"
    "\n"
    "; functions definitions (1)\n"
    "\n"
    "(function_definition (\"export\")? @exported (\"comptime\")? @comptime (\"test\")? @test type: (_) @return.type\n"
    "  declarator: [\n"
    "    (\n"
    "      function_declarator declarator: (identifier) @name)\n"
    "    (pointer_declarator (\"*\") @pointer.level declarator: (\n"
    "      function_declarator declarator: (identifier) @name))\n"
    "    (pointer_declarator (\"*\") @pointer.level declarator: (pointer_declarator (\"*\") @pointer.level2 declarator: (\n"
    "      function_declarator declarator: (identifier) @name)))\n"
    "    (pointer_declarator (\"*\") @pointer.level declarator: (pointer_declarator (\"*\") @pointer.level2 (pointer_declarator (\"*\") @pointer.level3 declarator: (\n"
    "      function_declarator declarator: (identifier) @name))))\n"
    "  ]\n"
    "  (_) @body) @definition.function\n"
    "\n";

  // printf("k_queryStr: %s\n", k_queryStr);

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));

  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    if (
      HashMap__contains(cursor->captureMap, "definition.function") &&
      HashMap__contains(cursor->captureMap, "name")
    ) {
      NodeData* mainNode = HashMap__get(cursor->captureMap, "definition.function");
      NodeData* nameNode = HashMap__get(cursor->captureMap, "name");
      NodeData* comptimeNode = HashMap__get(cursor->captureMap, "comptime");
      NodeData* exportedNode = HashMap__get(cursor->captureMap, "exported");

      const char* nameStrPtr = parsedFileContent + nameNode->startPos.index;
      const unsigned int nameStrLen = nameNode->endPos.index - nameNode->startPos.index;

      char* tmpFuncName = strndup(nameStrPtr, nameStrLen);
      // printf(" -{FUNC}-> {name=%s}\n", tmpFuncName);

      SourceIndexer__addFuncScope(self->indexer, tmpFuncName, mainNode->startPos, mainNode->endPos);

      if (comptimeNode) {
        SourceIndexer__addComptimeFunc(self->indexer, tmpFuncName);
      }

      if (self->fileType == SOURCE_H || (self->fileType == SOURCE_LC && exportedNode != NULL)) {
        SourceIndexer__addExportedDef(self->indexer, tmpFuncName);
      }

      free(tmpFuncName);
    }
    else if (
      HashMap__contains(cursor->captureMap, "block.scope")
    ) {
      NodeData* mainNode = HashMap__get(cursor->captureMap, "block.scope");

      // printf(" -{BLOCK}-> {---}\n");
      SourceIndexer__addBlockScope(self->indexer, mainNode->startPos, mainNode->endPos);
    }
  }

  SourceIndexer__computeScopesHierarchy(self->indexer);

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: queryStructDef
int AnalyzedFile__queryStructDef(AnalyzedFile *self)
{
  const char* k_queryStr = "\n"
    "\n"
    "; Structs, Unions, Enums, Typedefs\n"
    "\n"
    "(struct_specifier (\"export\")? @exported name: (type_identifier) @name templated_type: (type_identifier)? @templated.type (field_declaration_list) @field_declaration_list ) @definition.struct\n"
    "(enum_specifier (\"export\")? @exported name: (type_identifier) @name) @definition.enum\n"
    "(union_specifier (\"export\")? @exported name: (type_identifier) @name) @definition.union\n"
    "(type_definition (\"export\")? @exported declarator: (type_identifier) @name) @definition.typedef\n"
    "\n"
    "\n";

  // printf("k_queryStr: %s\n", k_queryStr);

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));

  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    if (
      HashMap__contains(cursor->captureMap, "definition.struct") ||
      HashMap__contains(cursor->captureMap, "definition.enum") ||
      HashMap__contains(cursor->captureMap, "definition.union") ||
      HashMap__contains(cursor->captureMap, "definition.typedef")
    ) {

      // NodeData* mainNode = HashMap__get(cursor->captureMap, "definition.struct");
      // if (!mainNode) { mainNode = HashMap__get(cursor->captureMap, "definition.enum"); }
      // if (!mainNode) { mainNode = HashMap__get(cursor->captureMap, "definition.union"); }
      // if (!mainNode) { mainNode = HashMap__get(cursor->captureMap, "definition.typedef"); }

      NodeData* nameNode = HashMap__get(cursor->captureMap, "name");

      {
        const char* pStrData = parsedFileContent + nameNode->startPos.index;
        const int strLen = (nameNode->endPos.index - nameNode->startPos.index);

        char* tmpName = strndup(pStrData, strLen);

        // printf(" -----> { %s }\n", tmpName);

        if (self->fileType == SOURCE_LC) {
          const NodeData* exportedNode = HashMap__get(cursor->captureMap, "exported");
          if (exportedNode) {
            SourceIndexer__addExportedDef(self->indexer, tmpName);
          }
        }
        else if (self->fileType == SOURCE_H) {
          SourceIndexer__addExportedDef(self->indexer, tmpName);
        }

        free(tmpName);
      }

    }

    // if (
    //   HashMap__contains(cursor->captureMap, "declaration.variable") &&
    //   HashMap__contains(cursor->captureMap, "type") &&
    //   HashMap__contains(cursor->captureMap, "name")
    // ) {
    //   NodeData* mainNode = HashMap__get(cursor->captureMap, "declaration.variable");
    //   NodeData* typeNode = HashMap__get(cursor->captureMap, "type");
    //   NodeData* nameNode = HashMap__get(cursor->captureMap, "name");

    //   const char* typeStr = parsedFileContent + typeNode->startPos.index;
    //   const unsigned int typeSize = typeNode->endPos.index - typeNode->startPos.index;
    //   char* tmpType = strndup(typeStr, typeSize);

    //   const char* nameStr = parsedFileContent + nameNode->startPos.index;
    //   const unsigned int nameSize = nameNode->endPos.index - nameNode->startPos.index;
    //   char* tmpName = strndup(nameStr, nameSize);

    //   // get the pointer level
    //   int pointerLevel = 0;
    //   for (unsigned int ii = 0; ii < nameSize; ++ii) {
    //     if (nameStr[ii] == '*') {
    //       ++pointerLevel;
    //     } else {
    //       break;
    //     }
    //   }
    //   if (pointerLevel > 0) {
    //     // isolate the variable name
    //     unsigned int ii = 0;
    //     for (; ii < nameSize; ++ii) {
    //       if (nameStr[ii] != '*' && nameStr[ii] != ' ' && nameStr[ii] != '\t') {
    //         break;
    //       }
    //     }
    //     free(tmpName);
    //     tmpName = strndup(nameStr + ii, nameSize - ii);
    //   }

    //   // printf(" -{DECL}-> {type=%s, name=%s, ptrLvl=%d}\n", tmpType, tmpName, pointerLevel);

    //   SourceIndexer__addVarDecl(self->indexer, tmpName, tmpType, pointerLevel, mainNode->startPos, mainNode->endPos);

    //   free(tmpType);
    //   free(tmpName);
    // }
  }

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: queryVarDef
int AnalyzedFile__queryVarDef(AnalyzedFile *self)
{
  const char* k_queryStr = "\n"
    "\n"
    "; var declaration\n"
    "\n"
    "(declaration (_) @type (identifier) @name (_)? @body) @declaration.variable\n"
    "(declaration (_) @type (init_declarator (identifier) @name (_)? @body)) @declaration.variable\n"
    "(declaration (_) @type (init_declarator (pointer_declarator)+ @name)) @declaration.variable\n"
    "\n"
    "; functions params\n"
    "\n"
    "(parameter_declaration (_) @type (identifier) @name (_)? @body) @declaration.variable\n"
    "(parameter_declaration (_) @type (pointer_declarator)+ @name) @declaration.variable\n"
    "\n";

  // printf("k_queryStr: %s\n", k_queryStr);

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));

  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    if (
      HashMap__contains(cursor->captureMap, "declaration.variable") &&
      HashMap__contains(cursor->captureMap, "type") &&
      HashMap__contains(cursor->captureMap, "name")
    ) {
      NodeData* mainNode = HashMap__get(cursor->captureMap, "declaration.variable");
      NodeData* typeNode = HashMap__get(cursor->captureMap, "type");
      NodeData* nameNode = HashMap__get(cursor->captureMap, "name");

      const char* typeStr = parsedFileContent + typeNode->startPos.index;
      const unsigned int typeSize = typeNode->endPos.index - typeNode->startPos.index;
      char* tmpType = strndup(typeStr, typeSize);

      const char* nameStr = parsedFileContent + nameNode->startPos.index;
      const unsigned int nameSize = nameNode->endPos.index - nameNode->startPos.index;
      char* tmpName = strndup(nameStr, nameSize);

      // get the pointer level
      int pointerLevel = 0;
      for (unsigned int ii = 0; ii < nameSize; ++ii) {
        if (nameStr[ii] == '*') {
          ++pointerLevel;
        } else {
          break;
        }
      }
      if (pointerLevel > 0) {
        // isolate the variable name
        unsigned int ii = 0;
        for (; ii < nameSize; ++ii) {
          if (nameStr[ii] != '*' && nameStr[ii] != ' ' && nameStr[ii] != '\t') {
            break;
          }
        }
        free(tmpName);
        tmpName = strndup(nameStr + ii, nameSize - ii);
      }

      // printf(" -{DECL}-> {type=%s, name=%s, ptrLvl=%d}\n", tmpType, tmpName, pointerLevel);

      SourceIndexer__addVarDecl(self->indexer, tmpName, tmpType, pointerLevel, mainNode->startPos, mainNode->endPos);

      free(tmpType);
      free(tmpName);
    }
  }

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: queryFuncCalls
int AnalyzedFile__queryFuncCalls(AnalyzedFile *self)
{
  const char* k_queryStr = "\n"
    "\n"
    "; function calls\n"
    "\n"
    "(call_expression function: ((identifier) @call.name (_) @call.args)) @call\n"
    "(call_expression function: (field_expression field: (field_identifier) @call.name) ((_) @call.args) ) @call\n"
    "\n";

  // printf("k_queryStr: %s\n", k_queryStr);

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));

  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    if (
      HashMap__contains(cursor->captureMap, "call") &&
      HashMap__contains(cursor->captureMap, "call.name")
    ) {
      NodeData* mainNode = HashMap__get(cursor->captureMap, "call");
      NodeData* nameNode = HashMap__get(cursor->captureMap, "call.name");

      const char* nameStr = parsedFileContent + nameNode->startPos.index;
      const unsigned int nameSize = nameNode->endPos.index - nameNode->startPos.index;
      char* tmpName = strndup(nameStr, nameSize);

      // printf(" -{CALL}-> {name=%s}\n", tmpName);

      SourceIndexer__addFunCallRef(self->indexer, tmpName, mainNode->startPos, mainNode->endPos);

      free(tmpName);
    }
  }

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: queryComptimeCalls
int AnalyzedFile__queryComptimeCalls(AnalyzedFile *self)
{
  const char* k_queryStr = "\n"
    "\n"
    "; comptime calls\n"
    "\n"
    "(comptime_call_expression function: ((identifier) @call.name arguments: (_) @call.args)) @comptime.call\n"
    "\n";

  // printf("k_queryStr: %s\n", k_queryStr);

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));


  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    if (
      HashMap__contains(cursor->captureMap, "comptime.call") &&
      HashMap__contains(cursor->captureMap, "call.name") &&
      HashMap__contains(cursor->captureMap, "call.args")
    ) {
      NodeData* mainNode = HashMap__get(cursor->captureMap, "comptime.call");
      NodeData* nameNode = HashMap__get(cursor->captureMap, "call.name");
      NodeData* argsNode = HashMap__get(cursor->captureMap, "call.args");

      const char* nameStr = parsedFileContent + nameNode->startPos.index;
      const unsigned int nameSize = nameNode->endPos.index - nameNode->startPos.index;
      char* tmpName = strndup(nameStr, nameSize);

      const char* argsStr = parsedFileContent + argsNode->startPos.index;
      const unsigned int argsSize = argsNode->endPos.index - argsNode->startPos.index;
      char* tmpArgs = strndup(argsStr, argsSize);

      // printf(" ---{COMPTIME-CALL}-> {name=%s}\n", tmpName);
      // printf("   -{COMPTIME-ARGS}-> {args=%s}\n", tmpArgs);

      SourceIndexer__addComptimeCall(self->indexer, tmpName, tmpArgs, mainNode->startPos, mainNode->endPos);

      free(tmpArgs);
      free(tmpName);
    }
  }

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: queryFuncSignatures
int AnalyzedFile__queryFuncSignatures(AnalyzedFile *self)
{
  if (self->fileType != SOURCE_H) {
    return 0;
  }

  const char* k_queryStr = "\n"
    "\n"
    "; functions signatures\n"
    "\n"
    // "(declaration (\"export\")? @exported (\"comptime\")? @comptime (\"test\")? @test type: (_) @return.type\n"
    "(declaration type: (_) @return.type\n"
    "  declarator: [\n"
    "    (\n"
    "      function_declarator declarator: (identifier) @name)\n"
    "    (pointer_declarator (\"*\") @pointer.level declarator: (\n"
    "      function_declarator declarator: (identifier) @name))\n"
    "    (pointer_declarator (\"*\") @pointer.level declarator: (pointer_declarator (\"*\") @pointer.level2 declarator: (\n"
    "      function_declarator declarator: (identifier) @name)))\n"
    "    (pointer_declarator (\"*\") @pointer.level declarator: (pointer_declarator (\"*\") @pointer.level2 (pointer_declarator (\"*\") @pointer.level3 declarator: (\n"
    "      function_declarator declarator: (identifier) @name))))\n"
    "  ]\n"
    "  ) @definition.function\n"
    "\n";

  // printf("k_queryStr: %s\n", k_queryStr);

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));


  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    if (
      HashMap__contains(cursor->captureMap, "definition.function") &&
      HashMap__contains(cursor->captureMap, "name")
    ) {
      // NodeData* mainNode = HashMap__get(cursor->captureMap, "definition.function");
      NodeData* nameNode = HashMap__get(cursor->captureMap, "name");

      const char* srcStr = parsedFileContent + nameNode->startPos.index;
      const unsigned int contentSize = nameNode->endPos.index - nameNode->startPos.index;

      char* tmpName = strndup(srcStr, contentSize);
      // printf(" -{FUNC-SIGN}-> {name=%s}\n", tmpName);

      // // SourceIndexer__addFuncScope(self->indexer, tmpName, mainNode->startPos, mainNode->endPos);
      // // TODO: is hacky
      // SourceIndexer__addVarRef(self->indexer, tmpName, mainNode->startPos, mainNode->endPos);

      SourceIndexer__addExportedDef(self->indexer, tmpName);

      free(tmpName);
    }
  }

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: queryVarRef
int AnalyzedFile__queryVarRef(AnalyzedFile *self)
{
  // if (self->fileType == SOURCE_H) {
  //   return 0;
  // }
  const char* k_queryStr = "\n"
    "\n"
    "; any identifier\n"
    "\n"
    "(identifier) @any.identifier\n"
    "\n";

  // printf("k_queryStr: %s\n", k_queryStr);

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));

  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    if (
      HashMap__contains(cursor->captureMap, "any.identifier")
    ) {
      NodeData* mainNode = HashMap__get(cursor->captureMap, "any.identifier");

      const char* nameStr = parsedFileContent + mainNode->startPos.index;
      const unsigned int nameSize = mainNode->endPos.index - mainNode->startPos.index;
      char* tmpName = strndup(nameStr, nameSize);

      // printf(" -{ID}-> {name=%s}\n", tmpName);

      SourceIndexer__addVarRef(self->indexer, tmpName, mainNode->startPos, mainNode->endPos);

      free(tmpName);
    }
  }

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: queryComptimeComments
int AnalyzedFile__queryComptimeComments(AnalyzedFile *self)
{
  if (self->fileType != SOURCE_LC) {
    // printf(" IS NOT A LC FILE -> {COMPTIME-COMMENt}\n");
    return 0;
  }

  // printf(" IS A LC FILE -> {COMPTIME-COMMENt}\n");

  const char* k_queryStr = "\n"
    "\n"
    // "; example -> ///comptime-lazy-c: \"text-replace\" \"HeapArena<int>\" \"HeapArena__int\"\n"
    // "\n"
    // "((comment) @comment.comptime (#match? @comment.comptime \"^///\\s*?comptime-lazy-c\\s*?\\:\\s*?.*$\"))\n"
    // "(comment) @comment.comptime\n"
    // "(comptime_comment) @comment.comptime\n"
    "(comment) @comment\n"
    "\n";

  const char* k_patternStr = "///comptime-lazy-c:";
  const unsigned int k_patternLen = strlen(k_patternStr);

  // printf("k_queryStr: %s\n", k_queryStr);

  const char* parsedFileContent = SourceParsedFile__getFileContent(self->parsedFile);
  QueryMatchData* newMatchData = SourceParsedFile__query(self->parsedFile, k_queryStr, strlen(k_queryStr));

  for (QueryMatchData* cursor = newMatchData; cursor; cursor = cursor->next)
  {
    NodeData* mainNode = HashMap__get(cursor->captureMap, "comment");
    if (!mainNode) {
      continue;
    }

    const char* nameStr = parsedFileContent + mainNode->startPos.index;
    const unsigned int nameSize = mainNode->endPos.index - mainNode->startPos.index;

    if (
      nameSize < k_patternLen ||
      strncmp(nameStr, k_patternStr, k_patternLen) != 0
    ) {
      continue;
    }

    char* tmpContent = strndup(nameStr, nameSize);

    // printf(" -{COMMENT}-> \"%s\"\n", tmpContent);
    // printf("   -{IS-COMPTIME!!!}-> --|%s|--\n", tmpContent + k_patternLen);

    const char* commandDblQuoteStart = strchr(tmpContent + k_patternLen, '\"');
    const char* commandDblQuoteEnd = strchr(commandDblQuoteStart + 1, '\"');

    const char* commandStr = commandDblQuoteStart + 1;
    const unsigned int commandLen = (int)(commandDblQuoteEnd - (commandDblQuoteStart + 1));

    char* tmpCmdStr = strndup(commandStr, commandLen);

    // printf("     -{CMD}-> --|%s|--\n", tmpCmdStr);

    // TODO: hashmap with
    if (strcmp(tmpCmdStr, "text-replace") == 0)
    {
      // printf("       -{TEXT-REPLACE}-\n");

      const char* arg1DblQuoteStart = strchr(commandDblQuoteEnd + 1, '\"');
      const char* arg1DblQuoteEnd = strchr(arg1DblQuoteStart + 1, '\"');
      const char* arg2DblQuoteStart = strchr(arg1DblQuoteEnd + 1, '\"');
      const char* arg2DblQuoteEnd = strchr(arg2DblQuoteStart + 1, '\"');

      const char* arg1Str = arg1DblQuoteStart + 1;
      const unsigned int arg1Len = (int)(arg1DblQuoteEnd - (arg1DblQuoteStart + 1));

      const char* arg2Str = arg2DblQuoteStart + 1;
      const unsigned int arg2Len = (int)(arg2DblQuoteEnd - (arg2DblQuoteStart + 1));

      char* tmpArg1Str = strndup(arg1Str, arg1Len);
      char* tmpArg2Str = strndup(arg2Str, arg2Len);

      // printf(" -> arg1=%s\n", tmpArg1Str);
      // printf(" -> arg2=%s\n", tmpArg2Str);

      SourceIndexer__addComptimeTextToReplace(self->indexer, tmpArg1Str, tmpArg2Str);

      free(tmpArg2Str);
      free(tmpArg1Str);
    }
    else if (strcmp(tmpCmdStr, "resolve-includes") == 0)
    {
      // printf("       -{RESOLVE-INCLUDE}-\n");

      const char* arg1DblQuoteStart = strchr(commandDblQuoteEnd + 1, '\"');
      const char* arg1DblQuoteEnd = strchr(arg1DblQuoteStart + 1, '\"');

      const char* arg1Str = arg1DblQuoteStart + 1;
      const unsigned int arg1Len = (int)(arg1DblQuoteEnd - (arg1DblQuoteStart + 1));

      char* tmpArg1Str = strndup(arg1Str, arg1Len);

      // printf(" -> arg1=%s\n", tmpArg1Str);

      if (
        strcmp(tmpArg1Str, "int") != 0 &&
        strcmp(tmpArg1Str, "float") != 0
      ) {

        printf("       -{RESOLVE-INCLUDE}-> %s\n", tmpArg1Str);

        SourceIndexer__addComptimeTypeToResolve(self->indexer, tmpArg1Str);
      }

      free(tmpArg1Str);
    }
    // else if (strcmp(tmpCmdStr, "resolve-type") == 0)
    // {
    //   // printf("       -{RESOLVE-TYPE}-\n");

    // }

    free(tmpCmdStr);

    free(tmpContent);
  }

  QueryMatchData__free(&newMatchData);
  return 0;
}

//MARK: getFilepath
const char *AnalyzedFile__getFilepath(const AnalyzedFile *self)
{
  return self->filepath;
}

//MARK: getIndexer
const SourceIndexer *AnalyzedFile__getIndexer(const AnalyzedFile *self)
{
  return self->indexer;
}

const SourceParsedFile* AnalyzedFile__getParsedFile(const AnalyzedFile *self)
{
  return self->parsedFile;
}

//MARK: debugTree
void AnalyzedFile__debugTree(const AnalyzedFile *self, StreamWriter *inStreamWriter)
{
  SourceParsedFile__debugTree(self->parsedFile, inStreamWriter);
}

//MARK: debugTree
void AnalyzedFile__debugScopeTree(const AnalyzedFile *self, StreamWriter *inStreamWriter)
{
  SourceIndexer__debugScopeTree(self->indexer, inStreamWriter);
}

