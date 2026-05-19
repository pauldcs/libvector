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
  (void)str_init(&s, allocator, 16);
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(str_is_empty(&s), true, "%d");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_NUM_EQUAL(((unsigned char *)s._vec._ptr)[0], (unsigned char)'\0',
                   "%u");
  str_deinit(&s);
  return (true);
}

static bool __test_002__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)str_init(&s, allocator, 0);
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  assert(s._vec._capacity > 0);
  str_deinit(&s);
  return (true);
}

static bool __test_003__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)str_init(&s, allocator, 16);
  assert(str_push_cstr(&s, strlen("hello"), (const unsigned char *)"hello"));
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)5, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)6, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "hello");
  ASSERT_NUM_EQUAL(((unsigned char *)s._vec._ptr)[5], (unsigned char)'\0',
                   "%u");
  str_deinit(&s);
  return (true);
}

static bool __test_004__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)str_init(&s, allocator, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"hello world"));
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)11, "%zu");
  ASSERT_NUM_EQUAL(str_is_empty(&s), false, "%d");
  str_clear(&s);
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(str_is_empty(&s), true, "%d");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_NUM_EQUAL(((unsigned char *)s._vec._ptr)[0], (unsigned char)'\0',
                   "%u");
  str_deinit(&s);
  return (true);
}

static bool __test_005__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)str_init(&s, allocator, 16);
  str_clear(&s);
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  str_deinit(&s);
  return (true);
}

static bool __test_006__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)str_init(&s, allocator, 16);
  assert(str_push_cstr(&s, strlen("first run"),
                       (const unsigned char *)"first run"));
  str_clear(&s);
  assert(str_push_cstr(&s, strlen("second"), (const unsigned char *)"second"));
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)6, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "second");
  str_deinit(&s);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)str_init(&s, allocator, 16);
  for (int i = 0; i < 10; i++) {
    assert(str_push_char(&s, 'a'));
  }
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)10, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "aaaaaaaaaa");
  str_clear(&s);
  ASSERT_NUM_EQUAL(str_length(&s), (size_t)0, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  str_deinit(&s);
  return (true);
}

static bool __test_008__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)str_init(&s, allocator, 16);
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  ASSERT_NUM_EQUAL(str_length(&s), strlen((char *)s._vec._ptr), "%zu");
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"abcdef"));
  ASSERT_NUM_EQUAL(str_length(&s), strlen((char *)s._vec._ptr), "%zu");
  str_deinit(&s);
  return (true);
}

static bool __test_009__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)str_init(&s, allocator, 16);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"data"));
  str_deinit(&s);
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
  (void)str_init(&s, allocator, 16);
  assert(s._vec._len >= 1);
  assert(str_push_cstr(&s, 1000, (const unsigned char *)"x"));
  assert(s._vec._len >= 1);
  str_clear(&s);
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  for (int i = 0; i < 5; i++) {
    assert(str_push_char(&s, (unsigned char)('a' + i)));
    assert(s._vec._len >= 1);
  }
  for (int i = 0; i < 5; i++) {
    assert(str_pop_char(&s) >= 0);
    assert(s._vec._len >= 1);
  }
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  str_deinit(&s);
  return (true);
}

TEST_FUNCTION void str_basics_specs(void) {
  __test_start__;
  run_test(&__test_001__, "init creates an empty string with one-byte vector");
  run_test(&__test_002__, "init with zero capacity hint uses default");
  run_test(&__test_003__, "vector length is one more than string length");
  run_test(&__test_004__, "clear resets to empty and preserves invariant");
  run_test(&__test_005__, "clear on an empty string is a no-op");
  run_test(&__test_006__, "reuse after clear");
  run_test(&__test_007__, "push then clear leaves empty c-string");
  run_test(&__test_008__, "str_length matches strlen of buffer");
  run_test(&__test_009__, "deinit zeroes the descriptor");
  run_test(&__test_010__, "vector length stays at least one throughout life");
  __test_end__;
}