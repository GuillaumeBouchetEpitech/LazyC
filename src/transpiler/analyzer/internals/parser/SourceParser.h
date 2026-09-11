
#pragma once

#include "./internals/SourceParsedFile.h"

typedef struct SourceParser SourceParser;

SourceParser* SourceParser__create();
void SourceParser__free(SourceParser** inParser);
SourceParsedFile* SourceParser__parse(SourceParser* inParser, const char* inFilepath);

