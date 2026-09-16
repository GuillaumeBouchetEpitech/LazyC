
#pragma once

#include "./internals/SourceParsedFile.h"

typedef struct SourceParser SourceParser;
typedef struct TSQuery TSQuery;

SourceParser* SourceParser__create();
void SourceParser__free(SourceParser** self);
SourceParsedFile* SourceParser__parse(SourceParser* self, const char* inFilepath);

TSQuery* SourceParser__parseQuery(SourceParser *self, const char* inAlias, const char* inQueryStr);
