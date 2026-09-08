
#include "sub-file.h"

#include <stdio.h>

void my_func()
{
  {
    int integerVal = 666;
  }

  const char* msg = "World";
  printf("Hello %s!\n", msg);

  {
    float floatVal = 777.777f;
  }
}
