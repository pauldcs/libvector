#include "vector.h"
#include "unit_tests.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const vector_allocator_t k_alloc = {
    .alloc = malloc,
    .release = free,
};

static bool __test_001__(void) {
  string src = {0};
  string dst = {0};
  (void)str_init(&src, k_alloc, 16);
  assert(str_push_cstr(&src, 1000, (const unsigned char *)"hello"));
  assert(str_clone(&src, &dst));
  ASSERT_NUM_EQUAL(str_length(&dst), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "hello");
  assert(str_push_char(&dst, '!'));
  ASSERT_STR_EQUAL((char *)src._vec._ptr, "hello");
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "hello!");
  str_deinit(&src);
  str_deinit(&dst);
  return (true);
}

static bool __test_002__(void) {
  string src = {0};
  string dst = {0};
  (void)str_init(&src, k_alloc, 16);
  assert(str_clone(&src, &dst));
  ASSERT_NUM_EQUAL(str_length(&dst), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(dst._vec._len, (size_t)1, "%zu");
  ASSERT_NUM_EQUAL(((unsigned char *)dst._vec._ptr)[0], (unsigned char)'\0',
                   "%u");
  str_deinit(&src);
  str_deinit(&dst);
  return (true);
}

static bool __test_003__(void) {
  string src = {0};
  string dst = {0};
  (void)str_init(&src, k_alloc, 16);
  assert(str_push_cstr(&src, 1000, (const unsigned char *)"hello"));
  assert(str_clone_slice(&src, &dst, 1, 4));
  ASSERT_NUM_EQUAL(str_length(&dst), (size_t)3, "%zu");
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "ell");
  str_deinit(&src);
  str_deinit(&dst);
  return (true);
}

static bool __test_004__(void) {
  string src = {0};
  string dst = {0};
  (void)str_init(&src, k_alloc, 16);
  assert(str_push_cstr(&src, 1000, (const unsigned char *)"hello"));
  assert(str_clone_slice(&src, &dst, -3, 5));
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "llo");
  str_deinit(&src);
  str_deinit(&dst);
  return (true);
}

static bool __test_005__(void) {
  string src = {0};
  string dst = {0};
  (void)str_init(&src, k_alloc, 16);
  assert(str_push_cstr(&src, 1000, (const unsigned char *)"hi"));
  assert(str_clone_slice(&src, &dst, -100, 100));
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "hi");
  str_deinit(&src);
  str_deinit(&dst);
  return (true);
}

static bool __test_006__(void) {
  string src = {0};
  string dst = {0};
  (void)str_init(&src, k_alloc, 16);
  assert(str_push_cstr(&src, 1000, (const unsigned char *)"hello"));
  assert(str_clone_slice(&src, &dst, 4, 1));
  ASSERT_NUM_EQUAL(str_length(&dst), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(str_is_empty(&dst), true, "%d");
  str_deinit(&src);
  str_deinit(&dst);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  assert(str_from_cstr(&s, k_alloc, 1000, (const unsigned char *)"world"));
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "world");
  str_deinit(&s);
  return (true);
}

static bool __test_008__(void) {
  string s = {0};
  assert(str_from_cstr(&s, k_alloc, 3, (const unsigned char *)"truncated"));
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)3, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "tru");
  str_deinit(&s);
  return (true);
}

static bool __test_009__(void) {
  string s = {0};
  assert(str_from_cstr(&s, k_alloc, 16, NULL));
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)0, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  str_deinit(&s);
  return (true);
}

static bool __test_010__(void) {
  string s = {0};
  assert(str_from_format(&s, k_alloc, 64, "user=%s id=%d", "ada", 42));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "user=ada id=42");
  ASSERT_NUM_EQUAL(str_length(&s), strlen("user=ada id=42"), "%zu");
  str_deinit(&s);
  return (true);
}

static bool __test_011__(void) {
  string s = {0};
  assert(str_from_format(&s, k_alloc, 5, "%s world", "hello"));
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hello");
  str_deinit(&s);
  return (true);
}

static bool __test_012__(void) {
  string s = {0};
  assert(str_from_format(&s, k_alloc, 16, "%s", ""));
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  str_deinit(&s);
  return (true);
}

static bool __test_013__(void) {
  string s = {0};
  const size_t cap = 16;
  unsigned char *buf;
  buf = (unsigned char *)k_alloc.alloc(cap);
  assert(buf != NULL);
  memcpy(buf, "adopt", 5);
  buf[5] = '\0';
  str_from_raw_parts(&s, k_alloc, buf, 5, cap);
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "adopt");
  assert(str_push_char(&s, '!'));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "adopt!");
  str_deinit(&s);
  return (true);
}

TEST_FUNCTION void str_construction2_specs(void) {
  __test_start__;
  run_test(&__test_001__, "clone produces an independent copy");
  run_test(&__test_002__, "clone of empty preserves invariant");
  run_test(&__test_003__, "clone_slice extracts a substring");
  run_test(&__test_004__, "clone_slice with negative start counts from end");
  run_test(&__test_005__, "clone_slice clamps out-of-range bounds");
  run_test(&__test_006__, "clone_slice with inverted bounds is empty");
  run_test(&__test_007__, "from_cstr copies a NUL-terminated literal");
  run_test(&__test_008__, "from_cstr truncates at max_len");
  run_test(&__test_009__, "from_cstr with NULL is an empty success");
  run_test(&__test_010__, "from_format renders arguments");
  run_test(&__test_011__, "from_format clips at max_len");
  run_test(&__test_012__, "from_format with empty output is well-formed");
  run_test(&__test_013__, "from_raw_parts adopts caller buffer");
  __test_end__;
}
