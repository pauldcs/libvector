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
  assert(vector_push(&v, __PTRIZE_ST32__(42)));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)1, "%zu");
  assert(*(int32_t *)vector_first_to_ptr(&v) == 42);
  assert(*(int32_t *)vector_last_to_ptr(&v) == 42);
  vector_deinit(&v);
  return (true);
}

static bool __test_002__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 4, NULL);
  for (int32_t i = 0; i < 1000; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)1000, "%zu");
  for (int32_t i = 0; i < 1000; i++) {
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
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  assert(vector_push(&v, __PTRIZE_ST32__(1)));
  assert(vector_push(&v, __PTRIZE_ST32__(2)));
  assert(vector_push(&v, __PTRIZE_ST32__(3)));
  vector_pop(&v);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)2, "%zu");
  assert(*(int32_t *)vector_last_to_ptr(&v) == 2);
  vector_pop(&v);
  vector_pop(&v);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(vector_is_empty(&v), true, "%d");
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
  assert(vector_pushf(&v, __PTRIZE_ST32__(3)));
  assert(vector_pushf(&v, __PTRIZE_ST32__(2)));
  assert(vector_pushf(&v, __PTRIZE_ST32__(1)));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)3, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 2);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 3);
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
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_pushf(&v, __PTRIZE_ST32__(i)));
  }
  for (int32_t i = 0; i < 5; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == 4 - i);
  }
  vector_popf(&v);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)4, "%zu");
  assert(*(int32_t *)vector_first_to_ptr(&v) == 3);
  vector_popf(&v);
  assert(*(int32_t *)vector_first_to_ptr(&v) == 2);
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
    if (i % 2 == 0) {
      assert(vector_push(&v, __PTRIZE_ST32__(i)));
    } else {
      assert(vector_pushf(&v, __PTRIZE_ST32__(i)));
    }
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)10, "%zu");
  assert(*(int32_t *)vector_first_to_ptr(&v) == 9);
  assert(*(int32_t *)vector_last_to_ptr(&v) == 8);
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
  for (int32_t i = 0; i < 10; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  while (!vector_is_empty(&v)) {
    vector_pop(&v);
  }
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
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 10; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  while (!vector_is_empty(&v)) {
    vector_popf(&v);
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  vector_deinit(&v);
  return (true);
}

static bool __test_009__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(uint8_t), 8, NULL);
  for (int32_t i = 0; i < 200; i++) {
    uint8_t val = (uint8_t)i;
    assert(vector_push(&v, __PTRIZE_UT8__(val)));
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)200, "%zu");
  for (int32_t i = 0; i < 200; i++) {
    assert(*(uint8_t *)vector_index_to_ptr(&v, (size_t)i) == (uint8_t)i);
  }
  vector_deinit(&v);
  return (true);
}

static bool __test_010__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int64_t), 4, NULL);
  for (int32_t i = 0; i < 50; i++) {
    int64_t value = (int64_t)i * 1000000000LL;
    assert(vector_push(&v, __PTRIZE_ST64__(value)));
  }
  for (int32_t i = 0; i < 50; i++) {
    int64_t expected = (int64_t)i * 1000000000LL;
    assert(*(int64_t *)vector_index_to_ptr(&v, (size_t)i) == expected);
  }
  vector_deinit(&v);
  return (true);
}

static bool __test_011__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i + 1)));
  }
  for (int32_t i = 0; i < 5; i++) {
    int32_t expected = 5 - i;
    assert(*(int32_t *)vector_last_to_ptr(&v) == expected);
    vector_pop(&v);
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  vector_deinit(&v);
  return (true);
}

static bool __test_012__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i + 1)));
  }
  for (int32_t i = 0; i < 5; i++) {
    int32_t expected = i + 1;
    assert(*(int32_t *)vector_first_to_ptr(&v) == expected);
    vector_popf(&v);
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  vector_deinit(&v);
  return (true);
}

TEST_FUNCTION void vector_push_pop_specs(void) {
  __test_start__;
  run_test(&__test_001__, "push one element");
  run_test(&__test_002__, "growth across a thousand pushes");
  run_test(&__test_003__, "pop reduces length");
  run_test(&__test_004__, "pushf prepends");
  run_test(&__test_005__, "popf removes front");
  run_test(&__test_006__, "interleaved push and pushf");
  run_test(&__test_007__, "drain with pop");
  run_test(&__test_008__, "drain with popf");
  run_test(&__test_009__, "push of byte-sized elements");
  run_test(&__test_010__, "push of int64 with large values");
  run_test(&__test_011__, "stack discipline via push and pop");
  run_test(&__test_012__, "queue discipline via push and popf");
  __test_end__;
}