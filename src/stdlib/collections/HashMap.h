
#pragma once

typedef struct HashMap HashMap;

typedef struct EntryItem
{
  char *key;
  void *value;
} EntryItem;

HashMap* HashMap__preAllocate(unsigned int capacity);
void HashMap__free(HashMap** self);
void HashMap__clear(HashMap* self);
int HashMap__set(HashMap* self, const char* key, void* value);
void* HashMap__get(const HashMap* self, const char* key);
int HashMap__contains(const HashMap* self, const char* key);
int HashMap__remove(HashMap* self, const char* key);

unsigned int HashMap__get_totalBuckets(const HashMap* self);
unsigned int HashMap__get_totalItems(const HashMap* self);
char **HashMap__get_allKeys(const HashMap *self, unsigned int *totalKeys);
EntryItem* HashMap__get_allItems(const HashMap* self, unsigned int* totalItems);

