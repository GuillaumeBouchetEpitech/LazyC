
#include "./HashSet.h"

#include "./HashMap.h"

#include <stdlib.h>

typedef struct HashSet
{
  HashMap *internalMap;
} HashSet;

HashSet *HashSet__preAllocate(unsigned int capacity)
{
  HashSet *self = calloc(1, sizeof(HashSet));
  if (!self)
  {
    return NULL;
  }

  self->internalMap = HashMap__preAllocate(capacity);
  if (!self->internalMap)
  {
    free(self->internalMap);
    return NULL;
  }

  return self;
}

void HashSet__free(HashSet **self)
{
  if (!self || !*self)
  {
    return;
  }
  HashMap__free(&(*self)->internalMap);
  free(*self);
  *self = NULL;
}

void HashSet__clear(HashSet* self)
{
  HashMap__clear(self->internalMap);
}

int HashSet__set(HashSet *self, const char *key)
{
  return HashMap__set(self->internalMap, key, NULL);
}

int HashSet__contains(const HashSet *self, const char *key)
{
  return HashMap__contains(self->internalMap, key);
}

int HashSet__remove(HashSet *self, const char *key)
{
  return HashMap__remove(self->internalMap, key);
}

unsigned int HashSet__get_totalBuckets(const HashSet *self)
{
  return HashMap__get_totalBuckets(self->internalMap);
}

unsigned int HashSet__get_totalItems(const HashSet *self)
{
  return HashMap__get_totalItems(self->internalMap);
}

char **HashSet__get_allKeys(const HashSet *self, unsigned int *totalKeys)
{
  return HashMap__get_allKeys(self->internalMap, totalKeys);
}
