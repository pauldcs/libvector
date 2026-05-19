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
  assert(vector_adjust_cap_if_full(&v, 100));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  vector_deinit(&v);
  return (true);
}

static bool __test_002__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  assert(vector_adjust_exact_cap_if_full(&v, 50));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)5, "%zu");
  for (int32_t i = 0; i < 5; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == i);
  }
  vector_deinit(&v);
  return (true);
}

static bool __test_003__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 64, NULL);
  for (int32_t i = 0; i < 4; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  assert(vector_shrink_to_fit(&v));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)4, "%zu");
  for (int32_t i = 0; i < 4; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == i);
  }
  vector_deinit(&v);
  return (true);
}

static bool __test_004__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 4, NULL);
  for (int32_t i = 0; i < 4; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  assert(vector_shrink_to_fit(&v));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)4, "%zu");
  for (int32_t i = 0; i < 4; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == i);
  }
  vector_deinit(&v);
  return (true);
}

static bool __test_005__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  size_t cap = 16 * sizeof(int32_t);
  int32_t *buf = malloc(cap);
  assert(buf != NULL);
  for (int32_t i = 0; i < 5; i++) {
    buf[i] = i + 100;
  }
  vector_from_raw_parts(&v, allocator, buf, sizeof(int32_t), 5, cap, NULL);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)5, "%zu");
  ASSERT_NUM_EQUAL(vector_elem_size(&v), sizeof(int32_t), "%zu");
  for (int32_t i = 0; i < 5; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == i + 100);
  }
  vector_deinit(&v);
  return (true);
}

static bool __test_006__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  size_t cap = 32 * sizeof(int32_t);
  int32_t *buf = malloc(cap);
  assert(buf != NULL);
  for (int32_t i = 0; i < 10; i++) {
    buf[i] = i;
  }
  vector_from_raw_parts(&v, allocator, buf, sizeof(int32_t), 10, cap, NULL);
  assert(vector_push(&v, __PTRIZE_ST32__(999)));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)11, "%zu");
  assert(*(int32_t *)vector_last_to_ptr(&v) == 999);
  vector_deinit(&v);
  return (true);
}

static bool __test_007__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 16, NULL);
  assert(vector_adjust_cap_if_full(&v, 0));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  assert(vector_adjust_exact_cap_if_full(&v, 0));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  vector_deinit(&v);
  return (true);
}

static bool __test_008__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 4, NULL);
  for (int32_t i = 0; i < 100; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  while (vector_length(&v) > 4) {
    vector_pop(&v);
  }
  assert(vector_shrink_to_fit(&v));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)4, "%zu");
  for (int32_t i = 0; i < 4; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == i);
  }
  vector_deinit(&v);
  return (true);
}

static bool __test_009__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 4, NULL);
  assert(vector_adjust_exact_cap_if_full(&v, 1000));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  for (int32_t i = 0; i < 1000; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)1000, "%zu");
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
  for (int32_t i = 0; i < 50; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  assert(vector_shrink_to_fit(&v));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)50, "%zu");
  for (int32_t i = 0; i < 50; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == i);
  }
  vector_deinit(&v);
  return (true);
}

TEST_FUNCTION void vector_capacity_specs(void) {
  __test_start__;
  run_test(&__test_001__, "adjust_cap_if_full grows capacity");
  run_test(&__test_002__, "adjust_exact_cap_if_full preserves contents");
  run_test(&__test_003__, "shrink_to_fit reduces oversized capacity");
  run_test(&__test_004__, "shrink_to_fit when already tight is a no-op");
  run_test(&__test_005__, "from_raw_parts adopts external buffer");
  run_test(&__test_006__, "from_raw_parts then push grows normally");
  run_test(&__test_007__, "adjust_cap with zero extra is harmless");
  run_test(&__test_008__, "pop down then shrink_to_fit");
  run_test(&__test_009__, "adjust_exact_cap reserves for bulk push");
  run_test(&__test_010__, "shrink_to_fit after large growth preserves data");
  __test_end__;
}