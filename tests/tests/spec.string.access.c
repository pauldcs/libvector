#include "vector.h"
#include "unit_tests.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const vector_allocator_t k_alloc = {
    .alloc = malloc,
    .release = free,
};

static bool __test_001__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hello"));
  unsigned char *p = str_index_to_ptr(&s, 1);
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'e', "%u");
  str_deinit(&s);
  return (true);
}

static bool __test_002__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  unsigned char *p = str_index_to_ptr(&s, (ssize_t)str_length(&s));
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'\0', "%u");
  str_deinit(&s);
  return (true);
}

static bool __test_003__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  assert(str_index_to_ptr(&s, 99) == NULL);
  str_deinit(&s);
  return (true);
}

static bool __test_004__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hello"));
  unsigned char *p = str_index_to_ptr(&s, -1);
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'o', "%u");
  str_deinit(&s);
  return (true);
}

static bool __test_005__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hello"));
  unsigned char *p = str_index_to_ptr(&s, -(ssize_t)str_length(&s));
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'h', "%u");
  assert(str_index_to_ptr(&s, -(ssize_t)str_length(&s) - 1) == NULL);
  str_deinit(&s);
  return (true);
}

static bool __test_006__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  unsigned char *p = str_index_to_ptr(&s, 0);
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'\0', "%u");
  assert(str_index_to_ptr(&s, 1) == NULL);
  assert(str_index_to_ptr(&s, -1) == NULL);
  str_deinit(&s);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hello"));
  unsigned char *p = str_index_to_ptr(&s, 0);
  assert(p != NULL);
  *p = (unsigned char)'H';
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "Hello");
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)5, "%zu");
  str_deinit(&s);
  return (true);
}

TEST_FUNCTION void str_access_specs(void) {
  __test_start__;
  run_test(&__test_001__, "positive index returns the matching byte");
  run_test(&__test_002__, "position == length returns the trailing NUL");
  run_test(&__test_003__, "position > length returns NULL");
  run_test(&__test_004__, "index -1 refers to the last byte");
  run_test(&__test_005__, "index -length is first, -length-1 is NULL");
  run_test(&__test_006__, "empty string: only position 0 is valid");
  run_test(&__test_007__, "writing through the pointer mutates in place");
  __test_end__;
}
