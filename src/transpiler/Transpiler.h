
#pragma once

typedef struct Transpiler Transpiler;

// forward declaration(s)
typedef struct PointerHeapArray PointerHeapArray;

Transpiler *Transpiler__create(
    const char *inBaseDir,
    const char *inEntryFilepath,
    const char *inOutputDir,
    PointerHeapArray* includePath,
    int handleTests);

void Transpiler__free(Transpiler **self);

int Transpiler__applyDebug(Transpiler* self);

// PointerHeapArray<cstring>
PointerHeapArray* Transpiler__getOutputSource(const Transpiler* self);
