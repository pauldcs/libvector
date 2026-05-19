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
  string copy = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello world"));
  assert(string_clone(&s, &copy));
  ASSERT_NUM_EQUAL(string_length(&copy), string_length(&s), "%zu");
  ASSERT_STR_EQUAL((char *)copy._vec._ptr, "hello world");
  string_deinit(&s);
  string_deinit(&copy);
  return (true);
}

static bool __test_002__(void) {
  string s = {0};
  string copy = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  assert(string_clone(&s, &copy));
  ASSERT_NUM_EQUAL(string_length(&copy), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&copy), true, "%d");
  ASSERT_NUM_EQUAL(copy._vec._len, (size_t)1, "%zu");
  string_deinit(&s);
  string_deinit(&copy);
  return (true);
}

static bool __test_003__(void) {
  string s = {0};
  string copy = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 16);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"original"));
  assert(string_clone(&s, &copy));
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"-edited"));
  ASSERT_STR_EQUAL((char *)copy._vec._ptr, "original");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "original-edited");
  string_deinit(&s);
  string_deinit(&copy);
  return (true);
}

static bool __test_004__(void) {
  string s = {0};
  string slice = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 32);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello world"));
  assert(string_clone_slice(&s, &slice, 0, 5));
  ASSERT_NUM_EQUAL(string_length(&slice), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)slice._vec._ptr, "hello");
  string_deinit(&s);
  string_deinit(&slice);
  return (true);
}

static bool __test_005__(void) {
  string s = {0};
  string slice = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 32);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello world"));
  assert(string_clone_slice(&s, &slice, 6, 11));
  ASSERT_NUM_EQUAL(string_length(&slice), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)slice._vec._ptr, "world");
  string_deinit(&s);
  string_deinit(&slice);
  return (true);
}

static bool __test_006__(void) {
  string s = {0};
  string slice = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 32);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hello world"));
  assert(string_clone_slice(&s, &slice, -5, 11));
  ASSERT_STR_EQUAL((char *)slice._vec._ptr, "world");
  string_deinit(&slice);
  assert(string_clone_slice(&s, &slice, 0, -6));
  ASSERT_STR_EQUAL((char *)slice._vec._ptr, "hello");
  string_deinit(&s);
  string_deinit(&slice);
  return (true);
}

static bool __test_007__(void) {
  string s = {0};
  string slice = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 32);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"hi"));
  assert(string_clone_slice(&s, &slice, 5, 10));
  ASSERT_NUM_EQUAL(string_length(&slice), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&slice), true, "%d");
  ASSERT_NUM_EQUAL(slice._vec._len, (size_t)1, "%zu");
  string_deinit(&s);
  string_deinit(&slice);
  return (true);
}

static bool __test_008__(void) {
  string s = {0};
  string slice = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)string_init(&s, allocator, 32);
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"abcdefg"));
  assert(string_clone_slice(&s, &slice, 5, 2));
  ASSERT_NUM_EQUAL(string_length(&slice), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(slice._vec._len, (size_t)1, "%zu");
  string_deinit(&s);
  string_deinit(&slice);
  return (true);
}

static bool __test_009__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  assert(string_from_cstr(&s, allocator, 1000,
                          (const unsigned char *)"from cstring"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)12, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "from cstring");
  string_deinit(&s);
  return (true);
}

static bool __test_010__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  assert(string_from_cstr(&s, allocator, 5,
                          (const unsigned char *)"truncate me please"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "trunc");
  string_deinit(&s);
  return (true);
}

static bool __test_011__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  assert(string_from_cstr(&s, allocator, 1000, NULL));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&s), true, "%d");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_012__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  assert(string_from_cstr(&s, allocator, 1000, (const unsigned char *)""));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  string_deinit(&s);
  return (true);
}

static bool __test_013__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  assert(string_from_format(&s, allocator, 1000, "%d + %d = %d", 2, 3, 5));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "2 + 3 = 5");
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)9, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_014__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  assert(string_from_format(&s, allocator, 1000, "%s/%s/%s", "path", "to",
                            "file.txt"));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "path/to/file.txt");
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)16, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_015__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  assert(string_from_format(&s, allocator, 5, "Hello, World"));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)5, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "Hello");
  string_deinit(&s);
  return (true);
}

static bool __test_016__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  assert(string_from_format(&s, allocator, 1000, ""));
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "");
  string_deinit(&s);
  return (true);
}

static bool __test_017__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  unsigned char *buf = malloc(32);
  assert(buf != NULL);
  memcpy(buf, "raw parts", 9);
  buf[9] = '\0';
  string_from_raw_parts(&s, allocator, buf, 9, 32);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)9, "%zu");
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "raw parts");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)10, "%zu");
  string_deinit(&s);
  return (true);
}

static bool __test_018__(void) {
  string s = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  unsigned char *buf = malloc(16);
  assert(buf != NULL);
  buf[0] = '\0';
  string_from_raw_parts(&s, allocator, buf, 0, 16);
  ASSERT_NUM_EQUAL(string_length(&s), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(s._vec._len, (size_t)1, "%zu");
  ASSERT_NUM_EQUAL(string_is_empty(&s), true, "%d");
  assert(string_push_cstr(&s, 1000, (const unsigned char *)"added"));
  ASSERT_STR_EQUAL((char *)s._vec._ptr, "added");
  string_deinit(&s);
  return (true);
}

TEST_FUNCTION void string_construction_specs(void) {
  __test_start__;
  run_test(&__test_001__, "clone deep-copies content");
  run_test(&__test_002__, "clone of empty string");
  run_test(&__test_003__, "clone is independent of source");
  run_test(&__test_004__, "clone_slice front prefix");
  run_test(&__test_005__, "clone_slice back suffix");
  run_test(&__test_006__, "clone_slice with negative indices");
  run_test(&__test_007__, "clone_slice past the end yields empty");
  run_test(&__test_008__, "clone_slice with end < start yields empty");
  run_test(&__test_009__, "from_cstr copies entire string");
  run_test(&__test_010__, "from_cstr truncates at max_len");
  run_test(&__test_011__, "from_cstr with NULL pointer yields empty");
  run_test(&__test_012__, "from_cstr with empty c-string yields empty");
  run_test(&__test_013__, "from_format renders integers");
  run_test(&__test_014__, "from_format renders strings");
  run_test(&__test_015__, "from_format truncates at max_len");
  run_test(&__test_016__, "from_format with empty format yields empty");
  run_test(&__test_017__, "from_raw_parts adopts external buffer");
  run_test(&__test_018__, "from_raw_parts with empty content");
  __test_end__;
}