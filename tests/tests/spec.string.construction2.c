#include "unit_tests.h"
#include "vector.h"
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
  (void)string_init(&src, k_alloc, 16);
  assert(string_push_cstr(&src, 1000, (const unsigned char *)"hello"));
  assert(string_clone(&src, &dst));
  ASSERT_NUM_EQUAL(string_length(&dst), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "hello");
  assert(string_push_char(&dst, '!'));
  ASSERT_STR_EQUAL((char *)src._vec._ptr, "hello");
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "hello!");
  string_deinit(&src);
  string_deinit(&dst);
  return (true);
}

static bool __test_002__(void) {
  string src = {0};
  string dst = {0};
  (void)string_init(&src, k_alloc, 16);
  assert(string_clone(&src, &dst));
  ASSERT_NUM_EQUAL(string_length(&dst), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(dst._vec._len, (size_t)1, "%zu");
  ASSERT_NUM_EQUAL(((unsigned char *)dst._vec._ptr)[0], (unsigned char)'\0',
                   "%u");
  string_deinit(&src);
  string_deinit(&dst);
  return (true);
}

static bool __test_003__(void) {
  string src = {0};
  string dst = {0};
  (void)string_init(&src, k_alloc, 16);
  assert(string_push_cstr(&src, 1000, (const unsigned char *)"hello"));
  assert(string_clone_slice(&src, &dst, 1, 4));
  ASSERT_NUM_EQUAL(string_length(&dst), (size_t)3, "%zu");
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "ell");
  string_deinit(&src);
  string_deinit(&dst);
  return (true);
}

static bool __test_004__(void) {
  string src = {0};
  string dst = {0};
  (void)string_init(&src, k_alloc, 16);
  assert(string_push_cstr(&src, 1000, (const unsigned char *)"hello"));
  assert(string_clone_slice(&src, &dst, -3, 5));
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "llo");
  string_deinit(&src);
  string_deinit(&dst);
  return (true);
}

static bool __test_005__(void) {
  string src = {0};
  string dst = {0};
  (void)string_init(&src, k_alloc, 16);
  assert(string_push_cstr(&src, 1000, (const unsigned char *)"hi"));
  assert(string_clone_slice(&src, &dst, -100, 100));
  ASSERT_STR_EQUAL((char *)dst._vec._ptr, "hi");
  string_deinit(&src);
  string_deinit(&dst);
  return (true);
}

static bool __test_006__(void) {
  string src = {0};
  string dst = {0};
  (void)string_init(&src, k_alloc, 16);
  assert(string_push_cstr(&src, 1000, (const unsigned char *)"hello"));
  assert(string_clone_slice(&src, &dst, 4, 1));
  ASSERT_NUM_EQUAL(string_length(&dst), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&dst), true, "%d");
  string_deinit(&src);
  string_deinit(&dst);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  assert(string_from_cstr(&s, k_alloc, 1000, (const unsigned char *)"world"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "world");
  string_deinit(&s);
  return (true);
}

static bool __test_008__(void) {
  string s = {0};
  assert(string_from_cstr(&s, k_alloc, 3, (const unsigned char *)"truncated"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)3, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "tru");
  string_deinit(&s);
  return (true);
}

static bool __test_009__(void) {
  string s = {0};
  assert(string_from_cstr(&s, k_alloc, 16, NULL));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  string_deinit(&s);
  return (true);
}

static bool __test_010__(void) {
  string s = {0};
  assert(string_from_format(&s, k_alloc, 64, "user=%s id=%d", "ada", 42));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "user=ada id=42");
  ASSERT_NUM_EQUAL(string_length(&s), strlen("user=ada id=42"), "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_011__(void) {
  string s = {0};
  assert(string_from_format(&s, k_alloc, 5, "%s world", "hello"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hello");
  string_deinit(&s);
  return (true);
}

static bool __test_012__(void) {
  string s = {0};
  assert(string_from_format(&s, k_alloc, 16, "%s", ""));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  string_deinit(&s);
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
  string_from_raw_parts(&s, k_alloc, buf, 5, cap);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "adopt");
  assert(string_push_char(&s, '!'));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "adopt!");
  string_deinit(&s);
  return (true);
}

TEST_FUNCTION void string_construction2_specs(void) {
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
