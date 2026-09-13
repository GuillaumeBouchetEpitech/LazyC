
#pragma once

#include "./internals/SourceParsedFile.h"

typedef struct SourceParser SourceParser;
typedef struct TSQuery TSQuery;

SourceParser* SourceParser__create();
void SourceParser__free(SourceParser** inParser);
SourceParsedFile* SourceParser__parse(SourceParser* inParser, const char* inFilepath);

TSQuery* SourceParser__getQueryA(SourceParser* inParser);
TSQuery* SourceParser__getQueryB(SourceParser* inParser);
TSQuery* SourceParser__getQueryC(SourceParser* inParser);

TSQuery* SourceParser__getQueryX(SourceParser* inParser);
