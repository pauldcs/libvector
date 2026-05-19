#include "internal.h"
#include "unit_tests.h"
#include "vector.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool __test_001__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(vector_elem_size(&v), sizeof(int32_t), "%zu");
  ASSERT_NUM_EQUAL(vector_is_empty(&v), true, "%d");
  vector_deinit(&v);
  return (true);
}

static bool __test_002__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 0, NULL);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(vector_is_empty(&v), true, "%d");
  assert(vector_push(&v, __PTRIZE_ST32__(42)));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)1, "%zu");
  vector_deinit(&v);
  return (true);
}

static bool __test_003__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int8_t), 4, NULL);
  ASSERT_NUM_EQUAL(vector_elem_size(&v), sizeof(int8_t), "%zu");
  vector_deinit(&v);

  (void)vector_init(&v, allocator, sizeof(int64_t), 4, NULL);
  ASSERT_NUM_EQUAL(vector_elem_size(&v), sizeof(int64_t), "%zu");
  vector_deinit(&v);

  (void)vector_init(&v, allocator, sizeof(double), 4, NULL);
  ASSERT_NUM_EQUAL(vector_elem_size(&v), sizeof(double), "%zu");
  vector_deinit(&v);
  return (true);
}

static bool __test_004__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)5, "%zu");
  ASSERT_NUM_EQUAL(vector_is_empty(&v), false, "%d");
  vector_clear(&v);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(vector_is_empty(&v), true, "%d");
  vector_deinit(&v);
  return (true);
}

static bool __test_005__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  vector_clear(&v);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(vector_is_empty(&v), true, "%d");
  vector_deinit(&v);
  return (true);
}

static bool __test_006__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 10; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  vector_clear(&v);
  for (int32_t i = 100; i < 110; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)10, "%zu");
  for (int32_t i = 0; i < 10; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == 100 + i);
  }
  vector_deinit(&v);
  return (true);
}

static bool __test_007__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  ASSERT_NUM_EQUAL(vector_size_of(&v), (size_t)0, "%zu");
  for (int32_t i = 0; i < 3; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  ASSERT_NUM_EQUAL(vector_size_of(&v), (size_t)3 * sizeof(int32_t), "%zu");
  vector_deinit(&v);
  return (true);
}

static bool __test_008__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i * 10)));
  }
  int32_t *first = (int32_t *)vector_first_to_ptr(&v);
  int32_t *third = (int32_t *)vector_index_to_ptr(&v, 2);
  int32_t *last = (int32_t *)vector_last_to_ptr(&v);
  ASSERT_NUM_EQUAL(vector_elem_get_offset(&v, first), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(vector_elem_get_offset(&v, third),
                   (size_t)2 * sizeof(int32_t), "%zu");
  ASSERT_NUM_EQUAL(vector_elem_get_offset(&v, last),
                   (size_t)4 * sizeof(int32_t), "%zu");
  vector_deinit(&v);
  return (true);
}

static bool __test_009__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  assert(*(int32_t *)vector_first_to_ptr(&v) == 0);
  assert(*(int32_t *)vector_last_to_ptr(&v) == 4);
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 0);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == 4);
  vector_deinit(&v);
  return (true);
}

static bool __test_010__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  ASSERT_NUM_EQUAL(vector_is_empty(&v), true, "%d");
  assert(vector_push(&v, __PTRIZE_ST32__(1)));
  ASSERT_NUM_EQUAL(vector_is_empty(&v), false, "%d");
  vector_pop(&v);
  ASSERT_NUM_EQUAL(vector_is_empty(&v), true, "%d");
  vector_deinit(&v);
  return (true);
}

TEST_FUNCTION void vector_basics_specs(void) {
  __test_start__;
  run_test(&__test_001__, "init creates an empty vector");
  run_test(&__test_002__, "init with zero capacity hint still allows push");
  run_test(&__test_003__, "init records element size for various types");
  run_test(&__test_004__, "clear empties");
  run_test(&__test_005__, "clear on an empty vector is a no-op");
  run_test(&__test_006__, "reuse after clear");
  run_test(&__test_007__, "size_of returns length times elem_size");
  run_test(&__test_008__, "elem_get_offset returns byte offset");
  run_test(&__test_009__, "first, last, and index accessors");
  run_test(&__test_010__, "is_empty tracks transitions");
  __test_end__;
}