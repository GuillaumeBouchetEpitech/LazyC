
#include "HashMap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
  char *key;
  void *value;
  struct Node *next;
} Node;

static void _Node_free(Node *node)
{
  free(node->key);
  free(node);
}

struct HashMap
{
  Node **allBuckets;
  unsigned int totalBuckets;
  unsigned int totalItems;
};

static uint64_t _hash_key(const char *key)
{
  uint64_t hashVal = 14695981039346656037UL;
  while (*key)
  {
    hashVal ^= (unsigned char)*key++;
    hashVal *= 1099511628211UL;
  }
  return hashVal;
}

static unsigned int _bucket_index(const HashMap *self, const char *key)
{
  return (unsigned int)(_hash_key(key) % self->totalBuckets);
}

static int _buckets_grow(HashMap *self)
{
  unsigned int nextCapacity = self->totalBuckets * 2;
  Node **nextBuckets = calloc(nextCapacity, sizeof(Node *));
  if (nextBuckets == NULL)
  {
    return -1;
  }
  for (unsigned int ii = 0; ii < self->totalBuckets; ++ii)
  {
    Node *node = self->allBuckets[ii];
    while (node != NULL)
    {
      Node *next = node->next;
      unsigned int idx = (unsigned int)(_hash_key(node->key) % nextCapacity);
      node->next = nextBuckets[idx];
      nextBuckets[idx] = node;
      node = next;
    }
  }
  free(self->allBuckets);
  self->allBuckets = nextBuckets;
  self->totalBuckets = nextCapacity;
  return 0;
}

static Node *_find_node(const HashMap *self, const char *key)
{
  unsigned int idx = _bucket_index(self, key);
  for (Node *node = self->allBuckets[idx]; node != NULL; node = node->next)
  {
    if (strcmp(node->key, key) == 0)
    {
      return node;
    }
  }
  return NULL;
}

HashMap *HashMap__preAllocate(unsigned int capacity)
{
  HashMap *map = calloc(1, sizeof(HashMap));
  if (map == NULL)
  {
    return NULL;
  }
  map->totalBuckets = capacity < 32 ? 32 : capacity;
  map->allBuckets = calloc(map->totalBuckets, sizeof(Node *));
  if (map->allBuckets == NULL)
  {
    free(map);
    return NULL;
  }
  return map;
}

void HashMap__free(HashMap **self)
{
  if (self == NULL || *self == NULL)
  {
    return;
  }
  for (unsigned int ii = 0; ii < (*self)->totalBuckets; ++ii)
  {
    Node *node = (*self)->allBuckets[ii];
    while (node != NULL)
    {
      Node *next = node->next;
      _Node_free(node);
      node = next;
    }
  }
  free((*self)->allBuckets);
  free(*self);
  *self = NULL;
}

void HashMap__clear(HashMap *self)
{
  if (self->totalItems == 0)
  {
    return;
  }

  for (unsigned int ii = 0; ii < self->totalBuckets; ++ii)
  {
    Node *node = self->allBuckets[ii];
    while (node != NULL)
    {
      Node *next = node->next;
      _Node_free(node);
      node = next;
    }
    self->allBuckets[ii] = NULL;
  }
  self->totalItems = 0;
}

int HashMap__set(HashMap *self, const char *key, void *inValuePtr)
{
  Node *existing = _find_node(self, key);
  if (existing != NULL)
  {
    // replace existing value
    existing->value = inValuePtr;
    return 0;
  }
  Node *newNode = calloc(1, sizeof(Node));
  if (newNode == NULL)
  {
    return -1;
  }
  newNode->key = strdup(key);
  if (!newNode->key)
  {
    free(newNode);
    return -1;
  }
  newNode->value = inValuePtr;
  const unsigned int idx = _bucket_index(self, key);
  // insert new node as the "head node" of the bucket
  newNode->next = self->allBuckets[idx];
  self->allBuckets[idx] = newNode;
  self->totalItems++;
  // check if the buckets memory need to grow
  if (self->totalItems * 4 >= self->totalBuckets * 3)
  {
    if (_buckets_grow(self) < 0)
    {
      return -1;
    }
  }
  return 0;
}

void *HashMap__get(const HashMap *self, const char *key)
{
  Node *node = _find_node(self, key);
  return node == NULL ? NULL : node->value;
}

int HashMap__contains(const HashMap *self, const char *key)
{
  return _find_node(self, key) != NULL ? 1 : 0;
}

int HashMap__remove(HashMap *self, const char *key)
{
  unsigned int idx = _bucket_index(self, key);
  Node **link = &self->allBuckets[idx];
  while (*link != NULL)
  {
    if (strcmp((*link)->key, key) == 0)
    {
      Node *node = *link;
      *link = node->next;
      _Node_free(node);
      self->totalItems--;
      return 1;
    }
    link = &(*link)->next;
  }
  return 0;
}

unsigned int HashMap__get_totalBuckets(const HashMap *self)
{
  return self->totalBuckets;
}

unsigned int HashMap__get_totalItems(const HashMap *self)
{
  return self->totalItems;
}

char **HashMap__get_allKeys(const HashMap *self, unsigned int *totalKeys)
{
  // is 1 element larger, extra element has the NULL value
  char **allKeys = calloc(self->totalItems + 1, sizeof(char *));
  if (!allKeys)
  {
    return NULL;
  }

  if (totalKeys)
  {
    *totalKeys = self->totalItems;
  }

  unsigned int currIndex = 0;
  for (unsigned int ii = 0; ii < self->totalBuckets; ++ii)
  {
    Node *cursor = self->allBuckets[ii];
    while (cursor != NULL)
    {
      allKeys[currIndex] = cursor->key;
      currIndex += 1;
      cursor = cursor->next;
    }
  }

  return allKeys;
}

EntryItem* HashMap__get_allItems(const HashMap* self, unsigned int* totalItems)
{
  // is 1 element larger, extra element has the NULL value
  EntryItem *allItems = calloc(self->totalItems + 1, sizeof(EntryItem));
  if (!allItems)
  {
    return NULL;
  }

  if (totalItems)
  {
    *totalItems = self->totalItems;
  }

  unsigned int currIndex = 0;
  for (unsigned int ii = 0; ii < self->totalBuckets; ++ii)
  {
    Node *cursor = self->allBuckets[ii];
    while (cursor != NULL)
    {
      allItems[currIndex].key = cursor->key;
      allItems[currIndex].value = cursor->value;
      currIndex += 1;
      cursor = cursor->next;
    }
  }

  return allItems;
}
