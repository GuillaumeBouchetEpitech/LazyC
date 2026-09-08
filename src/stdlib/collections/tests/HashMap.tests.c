
#include "../HashMap.h"

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

static void HashMap_can_create_pre_allocated_HashMap() {
  HashMap* testHashMap = HashMap__preAllocate(32);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  HashMap__free(&testHashMap);
  assert(testHashMap == NULL);
}

static void HashMap_can_insert_and_grow_internal_buckets_buffer_and_then_clear_it_all() {
  char buffer[512];
  HashMap* testHashMap = HashMap__preAllocate(32);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    HashMap__set(testHashMap, buffer, (void*)1);
  }
  assert(HashMap__get_totalItems(testHashMap) == 24);
  assert(HashMap__get_totalBuckets(testHashMap) == 64);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    assert(HashMap__contains(testHashMap, buffer) == 1);
    assert(HashMap__get(testHashMap, buffer) == (void*)1);
  }
  assert(HashMap__get_totalItems(testHashMap) == 24);
  assert(HashMap__get_totalBuckets(testHashMap) == 64);
  HashMap__clear(testHashMap);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 64);
  HashMap__free(&testHashMap);
  assert(testHashMap == NULL);
}

static void HashMap_can_find_that_return_NULL() {
  HashMap* testHashMap = HashMap__preAllocate(32);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  assert(HashMap__contains(testHashMap, "test1") == 0);
  assert(HashMap__get(testHashMap, "test1") == NULL);
  HashMap__free(&testHashMap);
  assert(testHashMap == NULL);
}

static void HashMap_can_insert_find_and_remove_one_value() {
  HashMap* testHashMap = HashMap__preAllocate(32);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  HashMap__set(testHashMap, "test1", (void*)1);
  assert(HashMap__get_totalItems(testHashMap) == 1);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  assert(HashMap__contains(testHashMap, "test1") == 1);
  assert(HashMap__get(testHashMap, "test1") == (void*)1);
  HashMap__remove(testHashMap, "test1");
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  HashMap__free(&testHashMap);
  assert(testHashMap == NULL);
}

static void HashMap_can_insert_and_grow_internal_buckets_buffer() {
  char buffer[512];
  HashMap* testHashMap = HashMap__preAllocate(32);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    HashMap__set(testHashMap, buffer, (void*)1);
  }
  assert(HashMap__get_totalItems(testHashMap) == 24);
  assert(HashMap__get_totalBuckets(testHashMap) == 64);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    assert(HashMap__contains(testHashMap, buffer) == 1);
    assert(HashMap__get(testHashMap, buffer) == (void*)1);
  }
  assert(HashMap__get_totalItems(testHashMap) == 24);
  assert(HashMap__get_totalBuckets(testHashMap) == 64);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    HashMap__remove(testHashMap, buffer);
  }
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 64);
  HashMap__free(&testHashMap);
  assert(testHashMap == NULL);
}

static void HashMap_can_replace_multiple_times_the_same_keys() {
  HashMap* testHashMap = HashMap__preAllocate(32);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  for (int ii = 0; ii < 24; ++ii) {
    HashMap__set(testHashMap, "my-test", (void*)1);
  }
  assert(HashMap__get_totalItems(testHashMap) == 1);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
    assert(HashMap__contains(testHashMap, "my-test") == 1);
    assert(HashMap__get(testHashMap, "my-test") == (void*)1);
  assert(HashMap__get_totalItems(testHashMap) == 1);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
    HashMap__remove(testHashMap, "my-test");
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  HashMap__free(&testHashMap);
  assert(testHashMap == NULL);
}

static void HashMap_can_get_all_the_keys() {
  char buffer[512];
  HashMap* testHashMap = HashMap__preAllocate(32);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 32);
  for (int ii = 0; ii < 24; ++ii) {
    snprintf(buffer, 512, "test-%d", ii + 1);
    HashMap__set(testHashMap, buffer, (void*)1);
  }
  assert(HashMap__get_totalItems(testHashMap) == 24);
  assert(HashMap__get_totalBuckets(testHashMap) == 64);

  unsigned int totalKeys;
  char** allkeys = HashMap__get_allKeys(testHashMap, &totalKeys);
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
  HashMap__clear(testHashMap);
  assert(HashMap__get_totalItems(testHashMap) == 0);
  assert(HashMap__get_totalBuckets(testHashMap) == 64);
  HashMap__free(&testHashMap);
  assert(testHashMap == NULL);
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

void HashMap_tests()
{
  printf("=== HashMap_tests ===\n\n");

  RUN_TEST(HashMap_can_create_pre_allocated_HashMap);
  RUN_TEST(HashMap_can_insert_and_grow_internal_buckets_buffer_and_then_clear_it_all);
  RUN_TEST(HashMap_can_find_that_return_NULL);
  RUN_TEST(HashMap_can_insert_find_and_remove_one_value);
  RUN_TEST(HashMap_can_insert_and_grow_internal_buckets_buffer);
  RUN_TEST(HashMap_can_replace_multiple_times_the_same_keys);
  RUN_TEST(HashMap_can_get_all_the_keys);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}
