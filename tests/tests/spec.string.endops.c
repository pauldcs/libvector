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
  ASSERT_NUM_EQUAL(string_pop_char(&s), -1, "%d");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_NUM_EQUAL(((unsigned char *)s._vec._ptr)[0], (unsigned char)'\0',
                   "%u");
  string_deinit(&s);
  return (true);
}

static bool __test_002__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  ASSERT_NUM_EQUAL(string_pop_char(&s), (int)'c', "%d");
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)2, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "ab");
  string_deinit(&s);
  return (true);
}

static bool __test_003__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, NULL));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  string_deinit(&s);
  return (true);
}

static bool __test_004__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 3, (const unsigned char *)"truncated"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)3, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "tru");
  string_deinit(&s);
  return (true);
}

static bool __test_005__(void) {
  string s = {0};
  const unsigned char src[] = {'a', 'b', '\0', 'c', 'd'};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, sizeof(src), src));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)2, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "ab");
  string_deinit(&s);
  return (true);
}

static bool __test_006__(void) {
  string a = {0};
  string b = {0};
  (void)string_init(&a, k_alloc, 16);
  (void)string_init(&b, k_alloc, 16);
  assert(string_push_cstr(&a, 1000, (const unsigned char *)"foo"));
  assert(string_push_cstr(&b, 1000, (const unsigned char *)"bar"));
  assert(string_append(&a, &b));
  ASSERT_STR_EQUAL((char *)a._vec._ptr, "foobar");
  ASSERT_STR_EQUAL((char *)b._vec._ptr, "bar");
  string_deinit(&a);
  string_deinit(&b);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"ab"));
  assert(string_append(&s, &s));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "abab");
  string_deinit(&s);
  return (true);
}

static bool __test_008__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"id="));
  assert(string_append_format(&s, 64, "%d", 7));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "id=7");
  ASSERT_NUM_EQUAL(string_length(&s), strlen("id=7"), "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_009__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  assert(string_pushf_char(&s, 'c'));
  assert(string_pushf_char(&s, 'b'));
  assert(string_pushf_char(&s, 'a'));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "abc");
  string_deinit(&s);
  return (true);
}

static bool __test_010__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  ASSERT_NUM_EQUAL(string_popf_char(&s), -1, "%d");
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  ASSERT_NUM_EQUAL(string_popf_char(&s), (int)'a', "%d");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "bc");
  string_deinit(&s);
  return (true);
}

static bool __test_011__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 8);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"["));
  assert(string_append_format(&s, 32, "%s", "INFO"));
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"] "));
  assert(string_append_format(&s, 32, "code=%d", 200));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "[INFO] code=200");
  ASSERT_NUM_EQUAL(string_length(&s), strlen((char *)s._vec._ptr), "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_012__(void) {
  string s = {0};
  (void)string_init(&s, k_alloc, 16);
  for (int i = 0; i < 32; i++) {
    assert(string_push_char(&s, (unsigned char)('a' + (i % 26))));
  }
  while (string_pop_char(&s) >= 0) {
    /* drain */
  }
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  string_deinit(&s);
  return (true);
}

TEST_FUNCTION void string_endops_specs(void) {
  __test_start__;
  run_test(&__test_001__, "pop on empty returns -1");
  run_test(&__test_002__, "pop returns last byte and shortens");
  run_test(&__test_003__, "push_cstr with NULL is a no-op success");
  run_test(&__test_004__, "push_cstr truncates at max_len");
  run_test(&__test_005__, "push_cstr stops at embedded NUL");
  run_test(&__test_006__, "append concatenates two strings");
  run_test(&__test_007__, "append onto self doubles the content");
  run_test(&__test_008__, "append_format builds onto existing content");
  run_test(&__test_009__, "pushf_char prepends in reverse order");
  run_test(&__test_010__, "popf_char on empty returns -1, else head byte");
  run_test(&__test_011__, "log-line build mixes push and append_format");
  run_test(&__test_012__, "push/pop round-trip restores empty state");
  __test_end__;
}
