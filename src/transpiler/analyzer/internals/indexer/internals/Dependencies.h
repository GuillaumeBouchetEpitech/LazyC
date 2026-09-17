
#include "stdlib/collections/HashSet.h"

typedef struct Dependencies
{
  HashSet *allFunctions;
  HashSet *allTypes;
  HashSet *provideFunctions;
  HashSet *provideTypes;
  HashSet *requireFunctions;
  HashSet *requireTypes;
}
Dependencies;

Dependencies Dependencies__create();
void Dependencies__free(Dependencies* self);

int Dependencies__require(const Dependencies* self, const Dependencies* other);

