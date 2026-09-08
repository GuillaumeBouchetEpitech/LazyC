
#pragma once

#include <stdlib.h>
#include "some-fake-include-file.h"

#define D_MYLOG(msg) printf("[MYLOG] %s\n", msg);
#define D_TEST_VAR 777

// struct, enum, union, typedef
export struct MyStruct
{
  int integerVal;
  float floatVal;
};

export enum MyEnum
{
  EnnumValA,
  EnnumValB
};

export union MyUnion
{
  int unionValA;
  float unionValB;
};

export typedef struct MyStruct MyStruct;

// signatures
void voidReturnFuncSignature(int param1);
int integerReturnFuncSignature(int param1);
int *intPtrReturnFuncSignature(int param1);
int **intPtrPtrReturnFuncSignature(int param1);

int main()
{

  int integerVar = 666;
  float floatVar = 666.666f;
  float *floatPtr1 = &floatVar;
  float **floatPtr2 = &floatPtr1;
  float ***floatPtr3 = &floatPtr2;

  voidReturnFuncSignature(777);
  integerReturnFuncSignature(777);
  intPtrReturnFuncSignature(777);
  intPtrPtrReturnFuncSignature(777);

  return EXIT_SUCCESS;
}

// definitions
void voidReturnFuncSignature(int param1)
{
  return;
}
int integerReturnFuncSignature(int param1)
{
  return 0;
}
int *intPtrReturnFuncSignature(int param1)
{
  return NULL;
}
int **intPtrPtrReturnFuncSignature(int param1)
{
  return NULL;
}
