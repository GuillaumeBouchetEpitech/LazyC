
#pragma once

struct testStructA
{
  int testFieldA_int;
  float testFieldA_float;
};

typedef struct testStructB
{
  int testFieldB_int;
  float testFieldB_float;
  struct testStructB* testFieldB_self;
  struct testStructA testFieldB_other;
}
testStructB;

union testUnionA {
  struct testStructA testUnionFieldA_structA;
  testStructB testUnionFieldA_structB;

  struct {
    int nestedFieldB_int;
    float nestedFieldB_float;
  }
  nestedField;

};

enum testEnumA {
  VAL1 = 0,
  VAL2,
  VAL3
};

typedef struct testStructA testStructA;
