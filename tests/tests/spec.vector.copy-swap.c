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
  int32_t src[] = {10, 20, 30, 40, 50};
  assert(vector_copy_contiguous(&v, 0, src, 5));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)5, "%zu");
  for (int32_t i = 0; i < 5; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == (i + 1) * 10);
  }
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
  int32_t src[] = {1, 2, 3, 4, 5};
  assert(vector_copy_contiguous(&v, 0, src, 5));
  int32_t mid[] = {77, 88};
  assert(vector_copy_contiguous(&v, 2, mid, 2));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)7, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 2);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 77);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 88);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == 3);
  assert(*(int32_t *)vector_index_to_ptr(&v, 5) == 4);
  assert(*(int32_t *)vector_index_to_ptr(&v, 6) == 5);
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
  int32_t src[] = {1, 2, 3};
  assert(vector_copy_contiguous(&v, 0, src, 3));
  int32_t more[] = {4, 5, 6};
  assert(vector_copy_contiguous(&v, 3, more, 3));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)6, "%zu");
  for (int32_t i = 0; i < 6; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == i + 1);
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
  int32_t big[200];
  for (int32_t i = 0; i < 200; i++) {
    big[i] = i * 2;
  }
  assert(vector_copy_contiguous(&v, 0, big, 200));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)200, "%zu");
  for (int32_t i = 0; i < 200; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == i * 2);
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
  vector_swap_elems(&v, 0, 4);
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 4);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == 0);
  vector_swap_elems(&v, 1, 3);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 3);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 2);
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
  vector_swap_elems(&v, 2, 2);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 2);
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)5, "%zu");
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
  for (int32_t i = 0; i < 6; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i)));
  }
  size_t half = vector_length(&v) / 2;
  for (size_t i = 0; i < half; i++) {
    vector_swap_elems(&v, i, vector_length(&v) - 1 - i);
  }
  for (int32_t i = 0; i < 6; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, (size_t)i) == 5 - i);
  }
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
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i + 1)));
  }
  int32_t extra = 99;
  assert(vector_push_within_inner(&v, &extra));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)6, "%zu");
  assert(*(int32_t *)vector_last_to_ptr(&v) == 99);
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
  for (int32_t i = 0; i < 5; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i + 1)));
  }
  int32_t extra = 99;
  assert(vector_insert_within_inner(&v, 2, &extra));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)6, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 99);
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
  int32_t init[] = {1, 2, 3, 4, 5};
  assert(vector_copy_contiguous(&v, 0, init, 5));
  int32_t more[] = {77, 88};
  assert(vector_copy_contiguous_within_inner(&v, 2, more, 2));
  ASSERT_NUM_EQUAL(vector_length(&v), (size_t)7, "%zu");
  assert(*(int32_t *)vector_index_to_ptr(&v, 0) == 1);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == 2);
  assert(*(int32_t *)vector_index_to_ptr(&v, 2) == 77);
  assert(*(int32_t *)vector_index_to_ptr(&v, 3) == 88);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == 3);
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
  for (int32_t i = 0; i < 6; i++) {
    assert(vector_push(&v, __PTRIZE_ST32__(i + 1)));
  }
  int32_t before[6];
  for (size_t i = 0; i < 6; i++) {
    before[i] = *(int32_t *)vector_index_to_ptr(&v, i);
  }
  vector_swap_elems(&v, 1, 4);
  assert(*(int32_t *)vector_index_to_ptr(&v, 1) == before[4]);
  assert(*(int32_t *)vector_index_to_ptr(&v, 4) == before[1]);
  vector_swap_elems(&v, 1, 4);
  for (size_t i = 0; i < 6; i++) {
    assert(*(int32_t *)vector_index_to_ptr(&v, i) == before[i]);
  }
  vector_deinit(&v);
  return (true);
}

TEST_FUNCTION void vector_copy_swap_specs(void) {
  __test_start__;
  run_test(&__test_001__, "copy_contiguous bulk fills from empty");
  run_test(&__test_002__, "copy_contiguous in the middle shifts the tail");
  run_test(&__test_003__, "successive copy_contiguous appends concatenate");
  run_test(&__test_004__, "copy_contiguous triggers growth");
  run_test(&__test_005__, "swap_elems exchanges two indices");
  run_test(&__test_006__, "swap_elems with the same index is a no-op");
  run_test(&__test_007__, "reverse via repeated swap_elems");
  run_test(&__test_008__, "push_within_inner uses preallocated capacity");
  run_test(&__test_009__, "insert_within_inner uses preallocated capacity");
  run_test(&__test_010__, "copy_contiguous_within_inner shifts tail");
  run_test(&__test_011__, "swap_elems is its own inverse");
  __test_end__;
}