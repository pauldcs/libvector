#include "unit_tests.h"
#include "vector.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool __test_001__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&s), true, "%d");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_NUM_EQUAL(((unsigned char *)s._vec._ptr)[0], (unsigned char)'\0',
                   "%u");
  string_deinit(&s);
  return (true);
}

static bool __test_002__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 0);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  assert(s._vec._capacity > 0);
  string_deinit(&s);
  return (true);
}

static bool __test_003__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  assert(string_push_cstr(&s, strlen("hello"), (const unsigned char *)"hello"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)5, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)6, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hello");
  ASSERT_NUM_EQUAL(((unsigned char *)s._vec._ptr)[5], (unsigned char)'\0',
                   "%u");
  string_deinit(&s);
  return (true);
}

static bool __test_004__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello world"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)11, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&s), false, "%d");
  string_clear(&s);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&s), true, "%d");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_NUM_EQUAL(((unsigned char *)s._vec._ptr)[0], (unsigned char)'\0',
                   "%u");
  string_deinit(&s);
  return (true);
}

static bool __test_005__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  string_clear(&s);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_006__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  assert(string_push_cstr(&s, strlen("first run"),
                          (const unsigned char *)"first run"));
  string_clear(&s);
  assert(
      string_push_cstr(&s, strlen("second"), (const unsigned char *)"second"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)6, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "second");
  string_deinit(&s);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  for (int i = 0; i < 10; i++) {
    assert(string_push_char(&s, 'a'));
  }
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)10, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "aaaaaaaaaa");
  string_clear(&s);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  string_deinit(&s);
  return (true);
}

static bool __test_008__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  ASSERT_NUM_EQUAL(string_length(&s), strlen((char *)s._vec._ptr), "%zu");
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abcdef"));
  ASSERT_NUM_EQUAL(string_length(&s), strlen((char *)s._vec._ptr), "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_009__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"data"));
  string_deinit(&s);
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._capacity, (size_t)0, "%zu");
  return (true);
}

static bool __test_010__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  assert(s._vec._len >= 1);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"x"));
  assert(s._vec._len >= 1);
  string_clear(&s);
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  for (int i = 0; i < 5; i++) {
    assert(string_push_char(&s, (unsigned char)('a' + i)));
    assert(s._vec._len >= 1);
  }
  for (int i = 0; i < 5; i++) {
    assert(string_pop_char(&s) >= 0);
    assert(s._vec._len >= 1);
  }
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  string_deinit(&s);
  return (true);
}

TEST_FUNCTION void string_basics_specs(void) {
  __test_start__;
  run_test(&__test_001__, "init creates an empty string with one-byte vector");
  run_test(&__test_002__, "init with zero capacity hint uses default");
  run_test(&__test_003__, "vector length is one more than string length");
  run_test(&__test_004__, "clear resets to empty and preserves invariant");
  run_test(&__test_005__, "clear on an empty string is a no-op");
  run_test(&__test_006__, "reuse after clear");
  run_test(&__test_007__, "push then clear leaves empty c-string");
  run_test(&__test_008__, "string_length matches strlen of buffer");
  run_test(&__test_009__, "deinit zeroes the descriptor");
  run_test(&__test_010__, "vector length stays at least one throughout life");
  __test_end__;
}