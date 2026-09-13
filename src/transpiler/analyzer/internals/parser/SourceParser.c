
#include "./SourceParser.h"

#include "stdlib/core/panic.h"
#include "stdlib/filesystem/readFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-lazyc.h>

typedef struct SourceParser
{
  TSParser *pParser;

  TSQuery *queryA;
  TSQuery *queryB;
  TSQuery *queryC;
  TSQuery *queryX;

} SourceParser;

SourceParser *SourceParser__create()
{
  SourceParser *newSrcParser = calloc(1, sizeof(SourceParser));
  if (!newSrcParser)
  {
    return NULL;
  }

  newSrcParser->pParser = ts_parser_new();
  if (
    !newSrcParser->pParser ||
    !ts_parser_set_language(newSrcParser->pParser, tree_sitter_lazyc())
  ) {
    fprintf(stderr, "grammar/runtime ABI mismatch\n");
    SourceParser__free(&newSrcParser);
    return NULL;
  }


  {

  // printf("   ---> query\n");
  // StopWatch stopWatch = StopWatch__create();
  // StopWatch__start(&stopWatch);

    const char* k_queryStr = "\n"
      "\n"
      "\n"
      "; includes\n"
      "\n"
      "(preproc_include path: (_) @import.source) @import\n"
      "\n"
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
      "\n"
      "\n"
      "; Structs, Unions, Enums, Typedefs\n"
      "\n"
      "(struct_specifier (\"export\")? @exported name: (type_identifier) @name templated_type: (type_identifier)? @templated.type (field_declaration_list) @field_declaration_list ) @definition.struct\n"
      "(enum_specifier (\"export\")? @exported name: (type_identifier) @name) @definition.enum\n"
      "(union_specifier (\"export\")? @exported name: (type_identifier) @name) @definition.union\n"
      "(type_definition (\"export\")? @exported declarator: (type_identifier) @name) @definition.typedef\n"
      "\n"
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
      "\n"
      "\n"
      "; function calls\n"
      "\n"
      "(call_expression function: ((identifier) @call.name (_) @call.args)) @call\n"
      "(call_expression function: (field_expression field: (field_identifier) @call.name) ((_) @call.args) ) @call\n"
      "\n"
      "\n"
      "; comptime calls\n"
      "\n"
      "(comptime_call_expression function: ((identifier) @call.name arguments: (_) @call.args)) @comptime.call\n"
      "\n"
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
      "\n"
      "\n"
      "\n";

    uint32_t err_offset;
    TSQueryError err_type;
    TSQuery *query = ts_query_new(tree_sitter_lazyc(), k_queryStr, (uint32_t)strlen(k_queryStr), &err_offset, &err_type);
    if (!query)
    {
      fprintf(stderr, "bad query (type %d) at byte %u\n", err_type, err_offset);
      panic("bad tree-sitter query");
      // return NULL;
    }
    newSrcParser->queryA = query;

  // StopWatch__stop(&stopWatch);
  // const double timeInSec = StopWatch__getTime(&stopWatch);
  // printf("     ---> %lf sec\n", timeInSec);
  // StopWatch__free(&stopWatch);

  }


  {
    const char* k_queryStr = "\n"
      "\n"
      "; any identifier\n"
      "\n"
      "(identifier) @any.identifier\n"
      "\n";

    uint32_t err_offset;
    TSQueryError err_type;
    TSQuery *query = ts_query_new(tree_sitter_lazyc(), k_queryStr, (uint32_t)strlen(k_queryStr), &err_offset, &err_type);
    if (!query)
    {
      fprintf(stderr, "bad query (type %d) at byte %u\n", err_type, err_offset);
      panic("bad tree-sitter query");
      // return NULL;
    }
    newSrcParser->queryB = query;
  }

  {
    const char* k_queryStr = "\n"
      "\n"
      "(comment) @comment\n"
      "\n";

    uint32_t err_offset;
    TSQueryError err_type;
    TSQuery *query = ts_query_new(tree_sitter_lazyc(), k_queryStr, (uint32_t)strlen(k_queryStr), &err_offset, &err_type);
    if (!query)
    {
      fprintf(stderr, "bad query (type %d) at byte %u\n", err_type, err_offset);
      panic("bad tree-sitter query");
      // return NULL;
    }
    newSrcParser->queryC = query;
  }


  {

    const char* k_queryStr = "\n"
      "\n"
      "; includes\n"
      "\n"
      "(preproc_include path: (_) @import.source) @import\n"
      "\n"
      "\n"
      "; functions definitions\n"
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
      "\n"
      "\n"
      "; Structs, Unions, Enums, Typedefs\n"
      "\n"
      "(struct_specifier (\"export\")? @exported name: (type_identifier) @name templated_type: (type_identifier)? @templated.type (field_declaration_list) @field_declaration_list ) @definition.struct\n"
      "(enum_specifier (\"export\")? @exported name: (type_identifier) @name) @definition.enum\n"
      "(union_specifier (\"export\")? @exported name: (type_identifier) @name) @definition.union\n"
      "(type_definition (\"export\")? @exported declarator: (type_identifier) @name) @definition.typedef\n"
      "\n"
      "\n"
      "; Super string literals\n"
      "\n"
      "(super_string_literal) @super.string.literal\n"
      "\n"
      "; Static method call\n"
      "\n"
      "(static_call_expression (statement_identifier) @namespace (call_expression function: ((identifier) @call.name (_) @call.args))) @static.call\n"
      "(static_call_expression (comptime_call_expression) @namespace (call_expression function: ((identifier) @call.name (_) @call.args))) @static.call\n"
      "\n"
      "; Non-Static method call\n"
      "\n"
      "(call_expression function: (field_expression argument: (identifier) @call.caller field: (field_identifier) @call.callee) ((_) @call.args) ) @method.call\n"
      "\n"
      "\n";

    uint32_t err_offset;
    TSQueryError err_type;
    TSQuery *query = ts_query_new(tree_sitter_lazyc(), k_queryStr, (uint32_t)strlen(k_queryStr), &err_offset, &err_type);
    if (!query)
    {
      fprintf(stderr, "bad query (type %d) at byte %u\n", err_type, err_offset);
      panic("bad tree-sitter query");
      // return NULL;
    }
    newSrcParser->queryX = query;
  }



  return newSrcParser;
}

void SourceParser__free(SourceParser **self)
{
  if (!self || !*self)
  {
    return;
  }

  // TODO: free all parsed files here?

  if ((*self)->pParser) {
    ts_parser_delete((*self)->pParser);
    (*self)->pParser = NULL;
  }
  free(*self);
  *self = NULL;
}

SourceParsedFile *SourceParser__parse(SourceParser *sefl, const char *inFilepath)
{
  StringData fileData;
  if (readFile(inFilepath, &fileData.data, &fileData.len) < 0)
  {
    return NULL;
  }

  TSTree *pTree = ts_parser_parse_string(sefl->pParser, NULL, fileData.data, (uint32_t)fileData.len);
  if (!pTree)
  {
    return NULL;
  }

  SourceParsedFile *newParsedFile = calloc(1, sizeof(SourceParsedFile));
  if (!newParsedFile)
  {
    return NULL;
  }

  newParsedFile->fileData.data = fileData.data;
  newParsedFile->fileData.len = fileData.len;
  newParsedFile->pTree = pTree;

  TSNode rootNode = ts_tree_root_node(pTree);
  const uint32_t startIndex = ts_node_start_byte(rootNode);
  const uint32_t endIndex = ts_node_end_byte(rootNode);
  const TSPoint startPoint = ts_node_start_point(rootNode);
  const TSPoint endPoint = ts_node_end_point(rootNode);

  newParsedFile->startPos.index = startIndex;
  newParsedFile->startPos.row = startPoint.row;
  newParsedFile->startPos.column = startPoint.column;

  newParsedFile->endPos.index = endIndex;
  newParsedFile->endPos.row = endPoint.row;
  newParsedFile->endPos.column = endPoint.column;

  return newParsedFile;
}

TSQuery* SourceParser__getQueryA(SourceParser* inParser)
{
  return inParser->queryA;
}
TSQuery* SourceParser__getQueryB(SourceParser* inParser)
{
  return inParser->queryB;
}
TSQuery* SourceParser__getQueryC(SourceParser* inParser)
{
  return inParser->queryC;
}

TSQuery* SourceParser__getQueryX(SourceParser* inParser)
{
  return inParser->queryX;
}
