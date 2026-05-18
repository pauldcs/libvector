#ifndef __STR_H__
#define __STR_H__

#include "vector.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct string {
  vector _vec;
} string;

bool str_init(string *str_uninit, vector_allocator_t allocator,
              size_t init_cap);
bool str_clone(const string *this, string *str_uninit);
bool str_clone_slice(const string *this, string *str_uninit, int64_t start,
                     int64_t end);
bool str_from_cstr(string *str_uninit, vector_allocator_t allocator,
                   size_t max_len, const unsigned char *cstr);
void str_from_raw_parts(string *str_uninit, vector_allocator_t allocator,
                        unsigned char *buf, size_t len, size_t capacity);
bool str_from_format(string *str_uninit, vector_allocator_t allocator,
                     size_t max_len, const char *format, ...);
void str_deinit(string *this);
size_t str_length(const string *this);
bool str_is_empty(const string *this);
void str_clear(string *this);
unsigned char *str_index_to_ptr(const string *this, ssize_t position);
bool str_push_char(string *this, unsigned char c);
bool str_push_cstr(string *this, size_t max_len, const unsigned char *cstr);
bool str_append(string *this, const string *other);
bool str_append_format(string *this, size_t max_len, const char *format, ...);
int str_pop_char(string *this);
bool str_pushf_char(string *this, unsigned char c);
int str_popf_char(string *this);
bool str_insert_char(string *this, size_t position, unsigned char c);
bool str_insert_cstr(string *this, size_t position, size_t max_len,
                     const unsigned char *cstr);
bool str_insert_other(string *this, size_t position, const string *other);
bool str_insert_format(string *this, size_t position, size_t max_len,
                       const char *format, ...);
void str_remove(string *this, size_t position);
void str_remove_range(string *this, size_t start, size_t end);
void str_truncate(string *this, size_t len);
bool str_shrink_to_fit(string *this);
unsigned char *str_leak_cstr(string *this);

#endif /* __STR_H__ */