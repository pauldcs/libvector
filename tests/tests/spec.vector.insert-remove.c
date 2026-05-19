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
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i + 1)));
  }
  assert(vector_insert(&v, 2, __PTRIZE_ST32__(99)));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)6, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 2);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 99);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 3);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == 4);
  assert(*(int32_t *)vector_index_to_ptr(&v, 5) == 5);
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
  assert(vector_insert(&v, 0, __PTRIZE_ST32__(99)));
  assert(*(int32_t *)vector_first_to_ptr(&v) == 99);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)6, "%zu");
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
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  assert(!vector_insert(&v, vector_length(&v), __PTRIZE_ST32__(99)));
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
  assert(vector_push(&v, __PTRIZE_ST32__(0)));
  for (int32_t i = 1; i < 100; i++) {
    assert(vector_insert(&v, 0, __PTRIZE_ST32__(i)));
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)100, "%zu");
  for (int32_t i = 0; i < 100; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == 99 - i);
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
  (void)vector_init(&v, allocator, sizeof(int32_t), 8, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  vector_remove(&v, 2);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)4, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 0);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 3);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 4);
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
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  vector_remove(&v, 0);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)4, "%zu");
  assert(*(int32_t *)vector_first_to_ptr(&v) == 1);
  vector_remove(&v, vector_length(&v) - 1);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)3, "%zu");
  assert(*(int32_t *)vector_last_to_ptr(&v) == 3);
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
  for (int32_t i = 0; i < 3; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  vector_remove(&v, 99);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)3, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 0);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 2);
  vector_deinit(&v);
  return (true);
}

static bool __test_008__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 16, NULL);
  for (int32_t i = 0; i < 10; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)10, "%zu");
  vector_remove_range(&v, 2, 5);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)5, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 0);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 7);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 8);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == 9);
  vector_deinit(&v);
  return (true);
}

static bool __test_009__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 16, NULL);
  for (int32_t i = 0; i < 10; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  vector_remove_range(&v, 0, 10);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)0, "%zu");
  ASSERT_NUM_EQUAL(vector_is_empty(&v), true, "%d");
  vector_deinit(&v);
  return (true);
}

static bool __test_010__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 16, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i * 11)));
  }
  vector_leak(&v, 2);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)4, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 0);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 11);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 33);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 44);
  vector_deinit(&v);
  return (true);
}

static bool __test_011__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 16, NULL);
  for (int32_t i = 0; i < 8; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  vector_leak_range(&v, 2, 3);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)5, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 0);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 5);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 6);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == 7);
  vector_deinit(&v);
  return (true);
}

static bool __test_012__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 16, NULL);
  for (int32_t i = 0; i < 6; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  vector_remove(&v, 0);
  vector_remove(&v, 0);
  vector_remove(&v, vector_length(&v) - 1);
  vector_remove(&v, vector_length(&v) - 1);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)2, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 2);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 3);
  vector_deinit(&v);
  return (true);
}

static bool __test_013__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 16, NULL);
  for (int32_t i = 0; i < 4; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
    assert(vector_insert(&v, 0, __PTRIZE_ST32__(100 + i)));
  }
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)8, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 103);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 102);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 101);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 100);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == 0);
  assert(*(int32_t *)vector_index_to_ptr(&v, 5) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 6) == 2);
  assert(*(int32_t *)vector_index_to_ptr(&v, 7) == 3);
  vector_deinit(&v);
  return (true);
}

static bool __test_014__(void) {
  vector v = {0};
  vector_allocator_t allocator = {
      .alloc = malloc,
      .release = free,
  };
  (void)vector_init(&v, allocator, sizeof(int32_t), 16, NULL);
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  vector_remove_range(&v, 1, 3);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)2, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 0);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 4);
  vector_deinit(&v);
  return (true);
}

TEST_FUNCTION void vector_insert_remove_specs(void) {
  __test_start__;
  run_test(&__test_001__, "insert in the middle shifts the tail");
  run_test(&__test_002__, "insert at front");
  run_test(&__test_003__, "insert at end does not behave like push");
  run_test(&__test_004__, "many inserts at front reverse the input");
  run_test(&__test_005__, "remove in the middle compacts the tail");
  run_test(&__test_006__, "remove at front and back");
  run_test(&__test_007__, "remove out of range is a no-op");
  run_test(&__test_008__, "remove_range removes a contiguous span");
  run_test(&__test_009__, "remove_range covering the whole vector");
  run_test(&__test_010__, "leak removes without destructor");
  run_test(&__test_011__, "leak_range removes a span without destructor");
  run_test(&__test_012__, "remove at front and back combined");
  run_test(&__test_013__, "interleaved push and insert at front");
  run_test(&__test_014__, "remove_range of an interior span");
  __test_end__;
}