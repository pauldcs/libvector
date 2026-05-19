#include "vector.h"
#include "unit_tests.h"
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
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  assert(str_insert_char(&s, 1, 'X'));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "aXbc");
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)4, "%zu");
  str_deinit(&s);
  return (true);
}

static bool __test_002__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"bc"));
  assert(str_insert_char(&s, 0, 'a'));
  assert(str_insert_char(&s, str_length(&s), 'd'));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "abcd");
  str_deinit(&s);
  return (true);
}

static bool __test_003__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hi"));
  ASSERT_NUM_EQUAL(str_insert_char(&s, 99, 'X'), false, "%d");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hi");
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)2, "%zu");
  str_deinit(&s);
  return (true);
}

static bool __test_004__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hello world"));
  assert(
      str_insert_cstr(&s, 6, 1000, (const unsigned char *)"beautiful "));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hello beautiful world");
  str_deinit(&s);
  return (true);
}

static bool __test_005__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  assert(str_insert_cstr(&s, 1, 1000, NULL));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "abc");
  str_deinit(&s);
  return (true);
}

static bool __test_006__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"AC"));
  assert(str_insert_cstr(&s, 1, 1, (const unsigned char *)"BBBBB"));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "ABC");
  str_deinit(&s);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hi"));
  ASSERT_NUM_EQUAL(
      str_insert_cstr(&s, 99, 1000, (const unsigned char *)"X"),
      false, "%d");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hi");
  str_deinit(&s);
  return (true);
}

static bool __test_008__(void) {
  string a = {0};
  string b = {0};
  (void)str_init(&a, k_alloc, 16);
  (void)str_init(&b, k_alloc, 16);
  assert(str_push_cstr(&a, 1000, (const unsigned char *)"helloworld"));
  assert(str_push_cstr(&b, 1000, (const unsigned char *)", "));
  assert(str_insert_other(&a, 5, &b));
  ASSERT_STR_EQUAL((char *)a._vec._ptr, "hello, world");
  /* b is untouched */
  ASSERT_STR_EQUAL((char *)b._vec._ptr, ", ");
  str_deinit(&a);
  str_deinit(&b);
  return (true);
}

static bool __test_009__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"abc"));
  /* insert "abc" at offset 0 should yield "abcabc" */
  assert(str_insert_other(&s, 0, &s));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "abcabc");
  str_deinit(&s);
  return (true);
}

static bool __test_010__(void) {
  string a = {0};
  string b = {0};
  (void)str_init(&a, k_alloc, 16);
  (void)str_init(&b, k_alloc, 16);
  assert(str_push_cstr(&a, 1000, (const unsigned char *)"abc"));
  assert(str_insert_other(&a, 1, &b));
  ASSERT_STR_EQUAL((char *)a._vec._ptr, "abc");
  str_deinit(&a);
  str_deinit(&b);
  return (true);
}

static bool __test_011__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"[]"));
  assert(str_insert_format(&s, 1, 64, "id=%d", 7));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "[id=7]");
  ASSERT_NUM_EQUAL(str_length(&s), strlen("[id=7]"), "%zu");
  str_deinit(&s);
  return (true);
}

static bool __test_012__(void) {
  string s = {0};
  (void)str_init(&s, k_alloc, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hi"));
  ASSERT_NUM_EQUAL(str_insert_format(&s, 99, 16, "%d", 1), false, "%d");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hi");
  str_deinit(&s);
  return (true);
}

TEST_FUNCTION void str_insert_specs(void) {
  __test_start__;
  run_test(&__test_001__, "insert_char in the middle shifts the tail");
  run_test(&__test_002__, "insert_char at 0 prepends, at length appends");
  run_test(&__test_003__, "insert_char out of range is rejected");
  run_test(&__test_004__, "insert_cstr splices a literal");
  run_test(&__test_005__, "insert_cstr with NULL is a no-op success");
  run_test(&__test_006__, "insert_cstr truncates at max_len");
  run_test(&__test_007__, "insert_cstr out of range is rejected");
  run_test(&__test_008__, "insert_other splices another string");
  run_test(&__test_009__, "insert_other handles self-aliasing");
  run_test(&__test_010__, "insert_other with empty source is a no-op");
  run_test(&__test_011__, "insert_format places formatted bytes at offset");
  run_test(&__test_012__, "insert_format out of range is rejected");
  __test_end__;
}
