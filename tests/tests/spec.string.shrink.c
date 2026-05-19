#include "unit_tests.h"
#include "vector.h"
#include <assert.h>
#include <stdbool.h>
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
  string_remove(&s, 1);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hllo");
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)4, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_002__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  string_remove(&s, 0);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "bc");
  string_remove(&s, string_length(&s) - 1);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "b");
  string_deinit(&s);
  return (true);
}

static bool __test_003__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hi"));
  string_remove(&s, 99);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hi");
  string_deinit(&s);
  return (true);
}

static bool __test_004__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abcdef"));
  string_remove_range(&s, 1, 4);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "aef");
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)3, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_005__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abcdef"));
  string_remove_range(&s, 2, 999);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "ab");
  string_deinit(&s);
  return (true);
}

static bool __test_006__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abcdef"));
  string_remove_range(&s, 3, 3);
  string_remove_range(&s, 4, 1);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "abcdef");
  string_deinit(&s);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abcdef"));
  string_truncate(&s, 3);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)3, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "abc");
  string_deinit(&s);
  return (true);
}

static bool __test_008__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abcdef"));
  string_truncate(&s, 0);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&s), true, "%d");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  string_deinit(&s);
  return (true);
}

static bool __test_009__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  string_truncate(&s, 99);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "abc");
  string_deinit(&s);
  return (true);
}

static bool __test_010__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 4);
  /* grow well past the initial capacity */
  for (int i = 0; i < 64; i++) {
    assert(string_push_char(&s, 'x'));
  }
  const size_t big_cap = s._vec._capacity;
  /* shrink the visible content drastically */
  string_truncate(&s, 1);
  assert(string_shrink_to_fit(&s));
  assert(s._vec._capacity < big_cap);
  /* content still readable and NUL-terminated */
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "x");
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)1, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_011__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  /* fill until used >= half capacity (target = vec_len; shrink fires only
   * when target < cap / 2) */
  while (s._vec._len * 2 < s._vec._capacity) {
    assert(string_push_char(&s, 'x'));
  }
  const size_t cap_before = s._vec._capacity;
  const size_t len_before = string_length(&s);
  assert(string_shrink_to_fit(&s));
  ASSERT_NUM_EQUAL(s._vec._capacity, cap_before, "%zu");
  ASSERT_NUM_EQUAL(string_length(&s), len_before, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_012__(void) {
  string s = {0};
  unsigned char *buf = NULL;
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"leaked"));
  string_leak_cstr(&s, &buf);
  assert(buf != NULL);
  /* buffer remains NUL-terminated for the caller */
  ASSERT_STR_EQUAL((char *)buf, "leaked");
  /* descriptor was zeroed */
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._capacity, (size_t)0, "%zu");
  /* caller is now responsible for the storage */
  k_alloc.release(buf);
  return (true);
}

TEST_FUNCTION void string_shrink_specs(void) {
  __test_start__;
  run_test(&__test_001__, "remove deletes one byte at a middle position");
  run_test(&__test_002__, "remove at the ends behaves like pop and popf");
  run_test(&__test_003__, "remove out of range is a no-op");
  run_test(&__test_004__, "remove_range deletes a half-open span");
  run_test(&__test_005__, "remove_range clamps end beyond length");
  run_test(&__test_006__, "remove_range with end <= start is a no-op");
  run_test(&__test_007__, "truncate shortens and keeps NUL terminator");
  run_test(&__test_008__, "truncate to zero yields an empty string");
  run_test(&__test_009__, "truncate to length >= current is a no-op");
  run_test(&__test_010__, "shrink_to_fit reclaims unused capacity");
  run_test(&__test_011__, "shrink_to_fit is a no-op when at least half-full");
  run_test(&__test_012__,
           "leak_cstr transfers ownership and zeroes descriptor");
  __test_end__;
}
