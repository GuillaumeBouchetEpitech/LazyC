

#include "../PointerHeapArray.h"

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

static void PointerHeapArray_can_create_pre_allocated()
{
  PointerHeapArray *testPtrHeapArray = PointerHeapArray__preAllocate(32);
  assert(testPtrHeapArray->len == 0);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }
  PointerHeapArray__free(&testPtrHeapArray);
  assert(testPtrHeapArray == NULL);
}

static void PointerHeapArray_can_push_and_pop_back_data()
{
  PointerHeapArray *testPtrHeapArray = PointerHeapArray__preAllocate(32);
  assert(testPtrHeapArray->len == 0);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  PointerHeapArray__pushBack(testPtrHeapArray, (void *)1);
  assert(testPtrHeapArray->len == 1);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  assert(testPtrHeapArray->data[0] == (void *)1);
  for (unsigned int ii = 1; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }
  PointerHeapArray__popBack(testPtrHeapArray);
  assert(testPtrHeapArray->len == 0);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }
  PointerHeapArray__free(&testPtrHeapArray);
  assert(testPtrHeapArray == NULL);
}

static void PointerHeapArray_can_push_and_pop_front_data()
{
  PointerHeapArray *testPtrHeapArray = PointerHeapArray__preAllocate(32);
  assert(testPtrHeapArray->len == 0);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  PointerHeapArray__pushBack(testPtrHeapArray, (void *)1);
  PointerHeapArray__pushBack(testPtrHeapArray, (void *)1);
  PointerHeapArray__pushBack(testPtrHeapArray, (void *)1);
  assert(testPtrHeapArray->len == 3);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 3; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 3; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__pushFront(testPtrHeapArray, (void *)2);
  assert(testPtrHeapArray->len == 4);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 1; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)2);
  }
  for (unsigned int ii = 1; ii < 4; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 4; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__pushFront(testPtrHeapArray, (void *)2);
  assert(testPtrHeapArray->len == 5);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 2; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)2);
  }
  for (unsigned int ii = 2; ii < 5; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 5; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__pushFront(testPtrHeapArray, (void *)2);
  assert(testPtrHeapArray->len == 6);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 3; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)2);
  }
  for (unsigned int ii = 3; ii < 6; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 6; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__popFront(testPtrHeapArray);
  assert(testPtrHeapArray->len == 5);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 2; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)2);
  }
  for (unsigned int ii = 2; ii < 5; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 5; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__popFront(testPtrHeapArray);
  assert(testPtrHeapArray->len == 4);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 1; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)2);
  }
  for (unsigned int ii = 1; ii < 4; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 4; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__popFront(testPtrHeapArray);
  assert(testPtrHeapArray->len == 3);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 3; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 3; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__popBack(testPtrHeapArray);
  PointerHeapArray__popBack(testPtrHeapArray);
  PointerHeapArray__popBack(testPtrHeapArray);

  assert(testPtrHeapArray->len == 0);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }
  PointerHeapArray__free(&testPtrHeapArray);
  assert(testPtrHeapArray == NULL);
}

static void PointerHeapArray_can_insert_and_erase_data_at_index()
{
  PointerHeapArray *testPtrHeapArray = PointerHeapArray__preAllocate(32);
  assert(testPtrHeapArray->len == 0);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  PointerHeapArray__pushBack(testPtrHeapArray, (void *)1);
  PointerHeapArray__pushBack(testPtrHeapArray, (void *)1);
  PointerHeapArray__pushBack(testPtrHeapArray, (void *)1);
  PointerHeapArray__pushBack(testPtrHeapArray, (void *)1);
  assert(testPtrHeapArray->len == 4);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 4; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 4; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__insertAtIndex(testPtrHeapArray, 2, (void *)2);
  assert(testPtrHeapArray->len == 5);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 2; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 2; ii < 3; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)2);
  }
  for (unsigned int ii = 3; ii < 5; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 5; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__insertAtIndex(testPtrHeapArray, 2, (void *)2);
  assert(testPtrHeapArray->len == 6);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 2; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 2; ii < 4; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)2);
  }
  for (unsigned int ii = 4; ii < 6; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 6; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__eraseAtIndex(testPtrHeapArray, 2);
  assert(testPtrHeapArray->len == 5);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 2; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 2; ii < 3; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)2);
  }
  for (unsigned int ii = 3; ii < 5; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 5; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__eraseAtIndex(testPtrHeapArray, 2);
  assert(testPtrHeapArray->len == 4);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < 4; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == (void *)1);
  }
  for (unsigned int ii = 4; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }

  PointerHeapArray__popBack(testPtrHeapArray);
  PointerHeapArray__popBack(testPtrHeapArray);
  PointerHeapArray__popBack(testPtrHeapArray);
  PointerHeapArray__popBack(testPtrHeapArray);

  assert(testPtrHeapArray->len == 0);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);
  for (unsigned int ii = 0; ii < testPtrHeapArray->cap; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == NULL);
  }
  PointerHeapArray__free(&testPtrHeapArray);
  assert(testPtrHeapArray == NULL);
}

static int _sortCallback(void *left, void *right, void *userData)
{
  const int* leftData = left;
  const int* rightData = right;
  return *rightData - *leftData; // ascending order
}

static void PointerHeapArray_can_sort_data_with_custom_funcPtr()
{
  PointerHeapArray *testPtrHeapArray = PointerHeapArray__preAllocate(32);
  assert(testPtrHeapArray->len == 0);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);

  int inputData[10] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
  int expectedData[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  for (int ii = 0; ii < 10; ++ii)
  {
    PointerHeapArray__pushBack(testPtrHeapArray, &inputData[ii]);
  }

  assert(testPtrHeapArray->len == 10);
  assert(testPtrHeapArray->cap == 32);
  assert(testPtrHeapArray->data != NULL);

  for (int ii = 0; ii < 10; ++ii)
  {
    assert(testPtrHeapArray->data[ii] == &inputData[ii]);
  }

  int wasSorted = PointerHeapArray__sortFromCallback(testPtrHeapArray, &_sortCallback, NULL);

  for (long int ii = 0; ii < 10; ++ii)
  {
    assert(*((int*)testPtrHeapArray->data[ii]) == expectedData[ii]);
  }

  PointerHeapArray__free(&testPtrHeapArray);
  assert(testPtrHeapArray == NULL);
}

//
//
//

//
//
//

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name)                    \
  do                                  \
  {                                   \
    printf("  TEST: %s ... ", #name); \
    tests_run++;                      \
  } while (0)

#define PASS()        \
  do                  \
  {                   \
    tests_passed++;   \
    printf("PASS\n"); \
  } while (0)

#define RUN_TEST(test_func) \
  do                        \
  {                         \
    TEST(test_func);        \
    test_func();            \
    PASS();                 \
  } while (0)

void PointerHeapArray_test()
{
  printf("=== PointerHeapArray_test ===\n\n");

  RUN_TEST(PointerHeapArray_can_create_pre_allocated);
  RUN_TEST(PointerHeapArray_can_push_and_pop_back_data);
  RUN_TEST(PointerHeapArray_can_push_and_pop_front_data);
  RUN_TEST(PointerHeapArray_can_insert_and_erase_data_at_index);
  RUN_TEST(PointerHeapArray_can_sort_data_with_custom_funcPtr);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}
