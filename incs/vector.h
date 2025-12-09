#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

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
typedef struct vector string;

struct vector {
  void *_ptr;        /* A pointer to the start of the buffer */
  size_t _elem_size; /* The size of one element (in bytes) */
  size_t _len;       /* The number of elements in the buffer */
  size_t _capacity;  /* The size of the reserved memory block (in bytes) */
  void (*_destructor)(void *);   /* the element destructor function */
  vector_allocator_t _allocator; /* the memory allocator */
};

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
void vector_deinit_zeroized(vector *__this);
bool vector_adjust_cap_if_full(vector *this, size_t n);
bool vector_adjust_exact_cap_if_full(vector *this, size_t n);
bool vector_push(vector *this, const void *element);
bool vector_push_within_inner(vector *this, const void *element);
void vector_push_within_inner_unchecked(vector *this, const void *element);
void vector_pop(vector *__this);
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
void vector_remove_range(vector *this, size_t start, size_t end);
void vector_leak(vector *__this, size_t position);
void vector_leak_unchecked(vector *__this, size_t position);
void vector_leak_range(vector *__this, size_t start, size_t len);
void vector_leak_range_unchecked(vector *__this, size_t start, size_t len);
void vector_swap_elems(vector *this, size_t a, size_t b);
bool vector_shrink_to_fit(vector *this);
void vector_from_raw_parts(vector *uninit_vec, vector_allocator_t allocator,
                           void *ptr, size_t elem_size, size_t len,
                           size_t capacity, void (*destructor)(void *));
size_t vector_elem_get_offset(const vector *this, const void *element);
void *vector_uninitialized_data(const vector *this);
size_t vector_uninitialized_size_of(const vector *this);
size_t vector_uninitialized_length(const vector *this);
void *vector_index_to_ptr_unchecked(const vector *this, size_t position);
void *vector_first_to_ptr_unchecked(const vector *this);
void *vector_last_to_ptr_unchecked(const vector *this);
void vector_append_from_capacity(vector *this, size_t n);

/* String */
bool dstr_create(string *dstr_uninit, vector_allocator_t allocator, size_t len);
bool dstr_clone(const string *this, string *dstr_uninit);
bool dstr_clone_slice(const string *this, string *dstr_uninit, int64_t start,
                      int64_t end);
bool dstr_from_cstr(string *dstr_uninit, vector_allocator_t allocator,
                    size_t max_len, const unsigned char *cstr);
void dstr_from_cstr_unchecked(string *dstr_uninit, vector_allocator_t allocator,
                              const unsigned char *cstr, size_t capacity,
                              size_t len);
bool dstr_from_format(string *dstr_uninit, vector_allocator_t allocator,
                      size_t max_len, const char *format, ...);
size_t dstr_length(const string *this);
bool dstr_is_empty(const string *this);
void dstr_clear(string *this);
void dstr_kill(string *this);
bool dstr_pushf_char(string *this, unsigned char c);
void dstr_popf_char(string *this);
bool dstr_push_char(string *this, unsigned char c);
bool dstr_push_cstr(string *this, size_t max_len, const unsigned char *cstr);
bool dstr_push_other(string *this, size_t position, const string *other);
bool dstr_push_format(string *this, size_t position, size_t max_len,
                      const char *format, ...);
void dstr_pop_char(string *this);
bool dstr_insert_char(string *this, size_t position, unsigned char c);
bool dstr_insert_cstr(string *this, size_t position, size_t max_len,
                      const unsigned char *cstr);
bool dstr_insert_format(string *this, size_t position, size_t max_len,
                        const char *format, ...);
bool dstr_insert_other(string *this, size_t position, const string *other);
unsigned char *dstr_index_to_ptr(const string *this, ssize_t position);
void dstr_remove(string *this, size_t position);
void dstr_remove_range(string *this, size_t start, size_t end);
void dstr_truncate(string *this, size_t len);
bool dstr_shrink_to_fit(string *this);
unsigned char *dstr_leak_cstr(const string *this);

#endif /* __VECTOR_H__ */
