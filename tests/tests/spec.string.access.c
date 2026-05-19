#include "unit_tests.h"
#include "vector.h"
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
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello"));
  unsigned char *p = string_index_to_ptr(&s, 1);
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'e', "%u");
  string_deinit(&s);
  return (true);
}

static bool __test_002__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  unsigned char *p = string_index_to_ptr(&s, (ssize_t)string_length(&s));
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'\0', "%u");
  string_deinit(&s);
  return (true);
}

static bool __test_003__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  assert(string_index_to_ptr(&s, 99) == NULL);
  string_deinit(&s);
  return (true);
}

static bool __test_004__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello"));
  unsigned char *p = string_index_to_ptr(&s, -1);
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'o', "%u");
  string_deinit(&s);
  return (true);
}

static bool __test_005__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello"));
  unsigned char *p = string_index_to_ptr(&s, -(ssize_t)string_length(&s));
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'h', "%u");
  assert(string_index_to_ptr(&s, -(ssize_t)string_length(&s) - 1) == NULL);
  string_deinit(&s);
  return (true);
}

static bool __test_006__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  unsigned char *p = string_index_to_ptr(&s, 0);
  assert(p != NULL);
  ASSERT_NUM_EQUAL(*p, (unsigned char)'\0', "%u");
  assert(string_index_to_ptr(&s, 1) == NULL);
  assert(string_index_to_ptr(&s, -1) == NULL);
  string_deinit(&s);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello"));
  unsigned char *p = string_index_to_ptr(&s, 0);
  assert(p != NULL);
  *p = (unsigned char)'H';
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "Hello");
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)5, "%zu");
  string_deinit(&s);
  return (true);
}

TEST_FUNCTION void string_access_specs(void) {
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
