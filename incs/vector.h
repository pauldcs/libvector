#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#if defined(__cplusplus) && (__cplusplus >= 201703L)
// C++17 or later use [[nodiscard]]
#define __no_discard [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#define __no_discard __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
#define __no_discard _Check_return_
#else
#define __no_discard
#endif

#if defined(__GNUC__) || defined(__clang__)
#define __pure1 __attribute__((pure))
#elif defined(_MSC_VER)
#define __pure1
#else
#define __pure1
#endif

#ifndef DEBUG
#define __bug_if_fail__(expr)
#else
#define __bug_if_fail__(expr)                                                  \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "buggy condition in %s line %d, expected (%s)\n",        \
              __FILE__, __LINE__, #expr);                                      \
      raise(SIGSEGV);                                                          \
    };                                                                         \
  } while (0)
#endif

#define MIN_CAP 24

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#if __has_builtin(__builtin_memcpy)
#define __memcpy __builtin_memcpy
#else
#define __memcpy memcpy
#endif

#if __has_builtin(__builtin_memmove)
#define __memmove __builtin_memmove
#else
#define __memmove memmove
#endif

#if __has_builtin(__builtin_memset)
#define __memset __builtin_memset
#else
#define __memset memset
#endif

#ifndef BUILTIN_EXPECT_AVAILABLE
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))
#else
#define likely(x) x
#define unlikely(x) x
#endif

typedef struct {
  void *(*alloc)(size_t);
  void (*release)(void *);
} vector_allocator_t;

typedef struct vector vector;

struct vector {
  void *_ptr;        /* A pointer to the start of the buffer */
  size_t _elem_size; /* The size of one element (in bytes) */
  size_t _len;       /* The number of elements in the buffer */
  size_t _capacity;  /* The size of the reserved memory block (in bytes) */
  void (*_destructor)(void *);   /* the element destructor function */
  vector_allocator_t _allocator; /* the memory allocator */
};

void __vector_set_length_internal(vector *this, size_t len);
void __vector_set_ptr_internal(vector *this, void *ptr);
void __vector_set_capacity_internal(vector *this, size_t cap);

bool vector_init(vector *uninit_vec, vector_allocator_t allocator,
                 size_t elem_size, size_t init_cap, void (*destructor)(void *));
size_t vector_length(const vector *this);
size_t vector_elem_size(const vector *this);
size_t vector_capacity(const vector *this);
bool vector_is_empty(const vector *this);
bool vector_is_full(const vector *this);
size_t vector_size_of(const vector *this);
void vector_clear(vector *this);
void vector_deinit(vector *this);
void vector_deinit_zeroized(vector *this);
bool vector_adjust_cap_if_full(vector *this, size_t n);
bool vector_adjust_exact_cap_if_full(vector *this, size_t n);
bool vector_push(vector *this, const void *element);
void vector_push_infaillible(vector *this, const void *element);
bool vector_push_within_inner(vector *this, const void *element);
void vector_push_within_inner_unchecked(vector *this, const void *element);
void vector_pop(vector *this);
bool vector_insert(vector *this, size_t position, const void *element);
bool vector_insert_within_inner(vector *this, size_t position,
                                const void *element);
void vector_insert_within_inner_unchecked(vector *this, size_t position,
                                          const void *element);
bool vector_copy_contiguous(vector *this, size_t position, const void *src,
                            size_t n);
bool vector_copy_contiguous_within_inner(vector *this, size_t position,
                                         const void *src, size_t n);
void vector_copy_contiguous_within_inner_unchecked(vector *this,
                                                   size_t position,
                                                   const void *src, size_t n);
void *vector_index_to_ptr(const vector *this, size_t position);
void *vector_first_to_ptr(const vector *this);
void *vector_last_to_ptr(const vector *this);
bool vector_pushf(vector *this, const void *element);
bool vector_pushf_within_inner(vector *this, const void *element);
void vector_pushf_within_inner_unchecked(vector *this, const void *element);
void vector_popf(vector *this);
void vector_remove(vector *this, size_t position);
void vector_remove_range(vector *this, size_t start, size_t len);
void vector_leak(vector *this, size_t position);
void vector_leak_unchecked(vector *this, size_t position);
void vector_leak_range(vector *this, size_t start, size_t len);
void vector_leak_range_unchecked(vector *this, size_t start, size_t len);
void vector_swap_elems(vector *this, size_t a, size_t b);
bool vector_shrink_to_fit(vector *this);
void vector_from_raw_parts(vector *uninit_vec, vector_allocator_t allocator,
                           void *ptr, size_t elem_size, size_t len,
                           size_t capacity, void (*destructor)(void *));
void vector_from_raw_static_parts(vector *uninit_vec, void *ptr,
                                  size_t elem_size, size_t len, size_t capacity,
                                  void (*destructor)(void *));
size_t vector_elem_get_offset(const vector *this, const void *element);
size_t vector_elem_get_index(const vector *this, const void *element);
void *vector_uninitialized_data(const vector *this);
size_t vector_uninitialized_size_of(const vector *this);
size_t vector_uninitialized_length(const vector *this);
void *vector_index_to_ptr_unchecked(const vector *this, size_t position);
void *vector_first_to_ptr_unchecked(const vector *this);
void *vector_last_to_ptr_unchecked(const vector *this);
void vector_append_from_capacity(vector *this, size_t n);

typedef struct string {
  vector _vec;
} string;

bool string_init(string *string_uninit, vector_allocator_t allocator,
                 size_t init_cap);
bool string_clone(const string *this, string *string_uninit);
bool string_clone_slice(const string *this, string *string_uninit,
                        int64_t start, int64_t end);
bool string_from_cstr(string *string_uninit, vector_allocator_t allocator,
                      size_t max_len, const unsigned char *cstr);
void string_from_raw_parts(string *string_uninit, vector_allocator_t allocator,
                           unsigned char *buf, size_t len, size_t capacity);
void string_from_raw_static_parts(string *string_uninit, unsigned char *buf,
                                  size_t len, size_t capacity);
bool string_from_format(string *string_uninit, vector_allocator_t allocator,
                        size_t max_len, const char *format, ...);
void string_deinit(string *this);
size_t string_length(const string *this);
bool string_is_empty(const string *this);
void string_clear(string *this);
unsigned char *string_index_to_ptr(const string *this, ssize_t position);
bool string_push_char(string *this, unsigned char byte);
bool string_push_cstr(string *this, size_t max_len, const unsigned char *cstr);
bool string_append(string *this, const string *other);
bool string_append_format(string *this, size_t max_len, const char *format,
                          ...);
int string_pop_char(string *this);
bool string_pushf_char(string *this, unsigned char byte);
int string_popf_char(string *this);
bool string_insert_char(string *this, size_t position, unsigned char byte);
bool string_insert_cstr(string *this, size_t position, size_t max_len,
                        const unsigned char *cstr);
bool string_insert_other(string *this, size_t position, const string *other);
bool string_insert_format(string *this, size_t position, size_t max_len,
                          const char *format, ...);
void string_remove(string *this, size_t position);
void string_remove_range(string *this, size_t start, size_t end);
void string_truncate(string *this, size_t len);
bool string_shrink_to_fit(string *this);
void string_leak_cstr(string *this, unsigned char **out_buf);

#endif /* __VECTOR_H__ */
