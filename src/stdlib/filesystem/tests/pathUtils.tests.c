
#include "../pathUtils.h"

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

static void Path__can_get_a_dirname()
{

  typedef struct TestCase
  {
    const char *input;
    const char *expected;
  } TestCase;

  static TestCase testCases[] = {
      {"", "."},
      {"/", "/"},
      {"foo", "."},
      {"foo/", "."},
      {"foo//", "."},
      {".env", "."},
      {".", "."},
      {"..", "."},
      {"foo.", "."},
      {"file.tar.gz", "."},
      {"a/b/c.txt", "a/b"},
      {"/foo", "/"},
      {"//a", "//"},
      {"//a/b", "//a"},
      {"/foo/", "/"},
      {"a/b/", "a"},
      {NULL, NULL},
  };

  TestCase *cursor = testCases;
  while (cursor->input)
  {
    char *dirName = Path__dirname(cursor->input);
    assert(dirName != NULL);
    assert(strcmp(dirName, cursor->expected) == 0);
    free(dirName);
    ++cursor;
  }
}

static void Path__can_get_a_basename()
{

  typedef struct TestCase
  {
    const char *input;
    const char *expected;
  } TestCase;

  static TestCase testCases[] = {
      {"", ""},
      {"/", ""},
      {"//", ""},
      {"foo", "foo"},
      {"foo/", "foo"},
      {"foo//", "foo"},
      {".env", ".env"},
      {".", "."},
      {"..", ".."},
      {"foo.", "foo."},
      {"file.tar.gz", "file.tar.gz"},
      {"a/b/c.txt", "c.txt"},
      {"/foo", "foo"},
      {"//a", "a"},
      {"a/b/", "b"},
      {NULL, NULL},
  };

  TestCase *cursor = testCases;
  while (cursor->input)
  {
    char *baseName = Path__basename(cursor->input);
    assert(baseName != NULL);
    assert(strcmp(baseName, cursor->expected) == 0);
    free(baseName);
    ++cursor;
  }
}

static void Path__can_get_a_extname()
{

  typedef struct TestCase
  {
    const char *input;
    const char *expected;
  } TestCase;

  static TestCase testCases[] = {
      {"file.tar.gz", ".gz"},
      {"README", ""},
      {"foo.", "."},
      {"a/b/c.txt", ".txt"},
      {"..", ""},
      {".", ""},
      {".env", ""},
      {"index.html", ".html"},
      {"my.file.tar.gz", ".gz"},
      {"foo.txt/bar", ""},
      {"foo.txt/.hidden", ""},
      {"", ""},
      {"/", ""},
      {"a/b.c/d.e", ".e"},
      {NULL, NULL},
  };

  TestCase *cursor = testCases;
  while (cursor->input)
  {
    char *baseName = Path__extname(cursor->input);
    assert(baseName != NULL);
    assert(strcmp(baseName, cursor->expected) == 0);
    free(baseName);
    ++cursor;
  }
}

static void Path__can_get_normalized()
{

  typedef struct TestCase
  {
    const char *input;
    const char *expected;
  } TestCase;

  static TestCase testCases[] = {
      {"a/b", "a/b"},
      {"/a/b", "/a/b"},
      {"a/../b", "b"},
      {"a/./b", "a/b"},
      {"//a", "/a"},
      {"/../b", "/b"},
      {"a/..", "."},
      {"../a", "../a"},
      {"../..", "../.."},
      {"/..//..", "/"},
      {"", "."},
      {"//", "/"},
      {"a/", "a/"},
      {"a//", "a/"},
      {"a//b", "a/b"},
      {"///", "/"},
      {"a/../", "./"},
      {"a/b/c/d/e", "a/b/c/d/e"},
      {NULL, NULL},
  };

  TestCase *cursor = testCases;
  while (cursor->input)
  {
    char *normPath = Path__normalize(cursor->input);
    // printf(" -> %s -> %s == %s\n", cursor->input, normPath, cursor->expected);
    assert(normPath != NULL);
    assert(strcmp(normPath, cursor->expected) == 0);
    free(normPath);
    ++cursor;
  }
}

static void Path__can_get_joined()
{

  typedef struct join_case
  {
    size_t n;
    const char *args[5];
    const char *expected;
  } join_case;

  static join_case testCases[] = {
      {2, {"a", "b"}, "a/b"},
      {2, {"/a", "b"}, "/a/b"},
      {3, {"a", "..", "b"}, "b"},
      {3, {"a", ".", "b"}, "a/b"},
      {2, {"/", "a"}, "/a"},
      {2, {"/..", "b"}, "/b"},
      {2, {"a", ".."}, "."},
      {2, {"..", "a"}, "../a"},
      {2, {"..", ".."}, "../.."},
      {2, {"/..", "/.."}, "/"},
      {1, {""}, "."},
      {0, {"", "", ""}, "."},
      {2, {"a", ""}, "a"},
      {2, {"a", "/"}, "a/"},
      {2, {"a", "/b"}, "a/b"},
      {2, {"/", "/"}, "/"},
      {2, {"a", "../"}, "./"},
      {5, {"a", "b", "c", "d", "e"}, "a/b/c/d/e"},
      {1, {NULL}, NULL},
  };

  join_case *cursor = testCases;
  while (cursor->expected)
  {
    char *r;
    switch (cursor->n)
    {
    case 0:
      r = Path__join(0);
      break;
    case 1:
      r = Path__join(1, cursor->args[0]);
      break;
    case 2:
      r = Path__join(2, cursor->args[0], cursor->args[1]);
      break;
    case 3:
      r = Path__join(3, cursor->args[0], cursor->args[1], cursor->args[2]);
      break;
    case 4:
      r = Path__join(4, cursor->args[0], cursor->args[1], cursor->args[2],
                     cursor->args[3]);
      break;
    case 5:
      r = Path__join(5, cursor->args[0], cursor->args[1], cursor->args[2],
                     cursor->args[3], cursor->args[4]);
      break;
    default:
      r = NULL;
      break;
    }

    // CHECK_STR(r, cursor->expected);
    assert(r != NULL);
    assert(strcmp(r, cursor->expected) == 0);

    free(r);
    // }
    ++cursor;
  }
}

static void Path__can_get_relative()
{
  typedef struct TestCase
  {
    const char *from;
    const char *to;
    const char *expected;
  } TestCase;

  TestCase testCases[] = {
      {"/data/orandea/test/aaa", "/data/orandea/impl/bbb", "../../impl/bbb"},
      {"/a/b/c", "/a/b/d", "../d"},
      {"/a/b/c", "/a/b/d/e", "../d/e"},
      {"/a", "/b", "../b"},
      {"/foo/bar", "/foo/bar/baz", "baz"},
      {"/foo/bar/baz", "/foo/bar", ".."},
      {"/", "/foo", "foo"},
      {"/foo", "/", ".."},
      {"a", "a", ""},
      {"a/b", "a/c", "../c"},
      {
        "/tests-assets/simple/src/sub-folder",
        "/tests-assets/simple/src/sub-folder/with-generics/.generated/HeapArena.lc.HeapArena__int.h",
        "with-generics/.generated/HeapArena.lc.HeapArena__int.h"
      },
      {NULL, NULL, NULL},
  };

  TestCase *cursor = testCases;
  while (cursor->expected)
  {
    char *normPath = Path__relative(cursor->from, cursor->to);
    // printf(" -> %s + %s -> %s == %s\n", cursor->from, cursor->to, normPath, cursor->expected);
    assert(normPath != NULL);
    assert(strcmp(normPath, cursor->expected) == 0);
    free(normPath);
    ++cursor;
  }
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

void Path_tests()
{
  printf("=== Path_tests ===\n\n");

  RUN_TEST(Path__can_get_a_dirname);
  RUN_TEST(Path__can_get_a_basename);
  RUN_TEST(Path__can_get_a_extname);
  RUN_TEST(Path__can_get_normalized);
  RUN_TEST(Path__can_get_joined);
  RUN_TEST(Path__can_get_relative);

  printf("\n--- %d/%d tests passed ---\n\n", tests_passed, tests_run);
}
