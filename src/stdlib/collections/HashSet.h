
#pragma once

typedef struct HashSet HashSet;

HashSet *HashSet__preAllocate(unsigned int capacity);
void HashSet__free(HashSet **self);
void HashSet__clear(HashSet* self);
int HashSet__set(HashSet *self, const char *key);
int HashSet__contains(const HashSet *self, const char *key);
int HashSet__remove(HashSet *self, const char *key);

unsigned int HashSet__get_totalBuckets(const HashSet *self);
unsigned int HashSet__get_totalItems(const HashSet *self);
char **HashSet__get_allKeys(const HashSet *self, unsigned int *totalKeys);
