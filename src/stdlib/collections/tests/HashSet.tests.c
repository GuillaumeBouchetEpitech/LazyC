
#include "../HashSet.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

//
//
//

//
//
//

static void HashSet_can_create_pre_allocated_HashSet() {
  HashSet* testHashSet = HashSet__preAllocate(32);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  HashSet__free(&testHashSet);
  assert(testHashSet == NULL);
}

static void HashSet_can_insert_and_grow_internal_buckets_buffer_and_then_clear_it_all() {
  char buffer[512];
  HashSet* testHashSet = HashSet__preAllocate(32);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    HashSet__set(testHashSet, buffer);
  }
  assert(HashSet__get_totalItems(testHashSet) == 24);
  assert(HashSet__get_totalBuckets(testHashSet) == 64);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    assert(HashSet__contains(testHashSet, buffer) == 1);
  }
  assert(HashSet__get_totalItems(testHashSet) == 24);
  assert(HashSet__get_totalBuckets(testHashSet) == 64);
  HashSet__clear(testHashSet);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 64);
  HashSet__free(&testHashSet);
  assert(testHashSet == NULL);
}

static void HashSet_can_find_that_return_NULL() {
  HashSet* testHashSet = HashSet__preAllocate(32);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  assert(HashSet__contains(testHashSet, "test1") == 0);
  HashSet__free(&testHashSet);
  assert(testHashSet == NULL);
}

static void HashSet_can_insert_find_and_remove_one_value() {
  HashSet* testHashSet = HashSet__preAllocate(32);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  HashSet__set(testHashSet, "test1");
  assert(HashSet__get_totalItems(testHashSet) == 1);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  assert(HashSet__contains(testHashSet, "test1") == 1);
  HashSet__remove(testHashSet, "test1");
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  HashSet__free(&testHashSet);
  assert(testHashSet == NULL);
}

static void HashSet_can_insert_and_grow_internal_buckets_buffer() {
  char buffer[512];
  HashSet* testHashSet = HashSet__preAllocate(32);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    HashSet__set(testHashSet, buffer);
  }
  assert(HashSet__get_totalItems(testHashSet) == 24);
  assert(HashSet__get_totalBuckets(testHashSet) == 64);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    assert(HashSet__contains(testHashSet, buffer) == 1);
  }
  assert(HashSet__get_totalItems(testHashSet) == 24);
  assert(HashSet__get_totalBuckets(testHashSet) == 64);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    HashSet__remove(testHashSet, buffer);
  }
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 64);
  HashSet__free(&testHashSet);
  assert(testHashSet == NULL);
}

static void HashSet_can_replace_multiple_times_the_same_keys() {
  HashSet* testHashSet = HashSet__preAllocate(32);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  for (int ii = 0; ii < 24; ++ii) {
    HashSet__set(testHashSet, "my-test");
  }
  assert(HashSet__get_totalItems(testHashSet) == 1);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
    assert(HashSet__contains(testHashSet, "my-test") == 1);
  assert(HashSet__get_totalItems(testHashSet) == 1);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
    HashSet__remove(testHashSet, "my-test");
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  HashSet__free(&testHashSet);
  assert(testHashSet == NULL);
}

static void HashSet_can_get_all_the_keys() {
  char buffer[512];
  HashSet* testHashSet = HashSet__preAllocate(32);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 32);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    HashSet__set(testHashSet, buffer);
  }
  assert(HashSet__get_totalItems(testHashSet) == 24);
  assert(HashSet__get_totalBuckets(testHashSet) == 64);

  unsigned int totalKeys;
  char** allkeys = HashSet__get_allKeys(testHashSet, &totalKeys);
  assert(totalKeys == 24);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);

    int wasFound = 0;
    for (int jj = 0; jj < totalKeys; ++jj) {
      if (strcmp(buffer, allkeys[jj]) == 0) {
        wasFound = 1;
        break;
      }
    }
    assert(wasFound == 1);
  }
  free(allkeys);
  HashSet__clear(testHashSet);
  assert(HashSet__get_totalItems(testHashSet) == 0);
  assert(HashSet__get_totalBuckets(testHashSet) == 64);
  HashSet__free(&testHashSet);
  assert(testHashSet == NULL);
}

//
//
//

//
//
//

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
  do { \
    printf("  TEST: %s ... ", #name); \
    tests_run++; \
  } while(0)

#define PASS() \
  do { \
    tests_passed++; \
    printf("PASS\n"); \
  } while(0)

#define RUN_TEST(test_func) \
  do { \
    TEST(test_func); \
    test_func(); \
    PASS(); \
  } while(0)

void HashSet_tests()
{
  printf("=== HashSet_tests ===\n\n");

  RUN_TEST(HashSet_can_create_pre_allocated_HashSet);
  RUN_TEST(HashSet_can_insert_and_grow_internal_buckets_buffer_and_then_clear_it_all);
  RUN_TEST(HashSet_can_find_that_return_NULL);
  RUN_TEST(HashSet_can_insert_find_and_remove_one_value);
  RUN_TEST(HashSet_can_insert_and_grow_internal_buckets_buffer);
  RUN_TEST(HashSet_can_replace_multiple_times_the_same_keys);
  RUN_TEST(HashSet_can_get_all_the_keys);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}
