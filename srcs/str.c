#include "str.h"
#include "vector.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Raw byte buffer of `s`.
 *
 * This essencially returns a pointer to the first byte.
 */
static inline __attribute__((always_inline)) unsigned char *
__str_ptr(const string *__this) {
  return ((unsigned char *)vector_first_to_ptr_unchecked(&__this->_vec));
}

/* Writes the trailing NUL at _ptr[_len].
 *
 * Note: caller must have reserved room for it.
 */
static inline __attribute__((always_inline)) void
__str_nul_terminate(string *__this) {
  __str_ptr(__this)[vector_length(&__this->_vec)] = '\0';
}

/* Sets _len and writes the trailing NUL.
 *
 * Note: caller must have reserved room for both.
 */
static inline __attribute__((always_inline)) void
__str_set_len_unchecked(string *__this, size_t len) {
  __vector_set_length_internal(&__this->_vec, len);
  __str_nul_terminate(__this);
}

/* Position of the first NUL in `s[0..max)`, else `max`.
 * Bounded read.
 */
static inline __attribute__((always_inline)) size_t
__str_strnlen(const unsigned char *s, size_t max) {
  size_t i = 0;
  while (i < max && s[i] != '\0') {
    ++i;
  }

  return (i);
}

/* `*out = a + b` with overflow check.
 *
 * Note: on overflow, *out is undefined.
 */
static inline __attribute__((always_inline)) bool
__str_add_size(size_t a, size_t b, size_t *out) {
  if (unlikely(a > SIZE_MAX - b)) {
    return (false);
  }

  *out = a + b;
  return (true);
}

/* Reserves room for `n` additional bytes plus the trailing NUL.
 */
static inline __attribute__((always_inline)) bool __str_reserve(string *__this,
                                                                size_t n) {
  size_t need;
  if (unlikely(!__str_add_size(n, 1, &need))) {
    return (false);
  }

  return (vector_adjust_cap_if_full(&__this->_vec, need));
}

/* Resolves a ssize_t index against `len`.
 *
 * Applies negative indexing (idx + len if idx < 0).
 * Result is clamped to [0, len].
 */
static inline __attribute__((always_inline)) size_t
__str_clamp_index(ssize_t idx, size_t len) {
  ssize_t signed_len = (ssize_t)len;
  if (idx < 0) {
    idx += signed_len;
    if (idx < 0) {
      idx = 0;
    }
  } else if (idx > signed_len) {
    idx = signed_len;
  }

  return ((size_t)idx);
}

/* Initialises the underlying vector with elem_size = 1.
 *
 * `init_cap` is in bytes.
 * 0 falls back to the default capacity (24).
 */
static inline __attribute__((always_inline)) bool
__str_init_internal(string *__this, vector_allocator_t allocator,
                    size_t init_cap) {
  return (vector_init(&__this->_vec, allocator, sizeof(unsigned char), init_cap,
                      NULL));
}

/* Probes vsnprintf for the byte count needed and clamps it to `max_len`.
 *
 * Returns false on encoding error, *out_want is unspecified in that case.
 * Note: does its own va_copy. The caller's args remain untouched and ready
 * for the actual vsnprintf write.
 */
static inline bool __str_format_probe(size_t max_len, const char *format,
                                      va_list args, size_t *out_want) {
  va_list probe;
  va_copy(probe, args);
  int probed = vsnprintf(NULL, 0, format, probe);
  va_end(probe);

  if (unlikely(probed < 0)) {
    return (false);
  }

  size_t want = (size_t)probed;
  if (want > max_len) {
    want = max_len;
  }

  *out_want = want;
  return (true);
}

/* Appends `n` bytes from `src` to the end of `this`.
 *
 * Maintains NUL.
 * Note: caller must have reserved capacity for n + 1.
 * Note: `src` must not alias `this->_vec._ptr`.
 */
static inline __attribute__((always_inline)) void
__str_append_bytes_unchecked(string *__this, const void *src, size_t n) {
  size_t len = vector_length(&__this->_vec);
  (void)memcpy(vector_index_to_ptr_unchecked(&__this->_vec, len), src, n);
  __str_set_len_unchecked(__this, len + n);
}

/* Inserts `n` bytes from `src` before position `position`.
 *
 * Maintains NUL.
 * Note: caller must have reserved capacity for n + 1.
 * Note: `position <= _len` is the only valid input.
 * Note: handles `src` aliasing `_ptr`.
 * Self insert with interior `position` still needs a temporary at the caller, see
 * str_insert_other.
 */
static inline __attribute__((always_inline)) void
__str_insert_bytes_unchecked(string *__this, size_t position, const void *src,
                             size_t n) {
  vector_copy_contiguous_within_inner_unchecked(&__this->_vec, position, src,
                                                n);
  __str_nul_terminate(__this);
}

/* Removes `n` bytes starting at `start`.
 *
 * Maintains NUL.
 * Note: caller must guarantee start + n <= _len.
 */
static inline __attribute__((always_inline)) void
__str_remove_bytes_unchecked(string *__this, size_t start, size_t n) {
  vector_leak_range_unchecked(&__this->_vec, start, n);
  __str_nul_terminate(__this);
}

/* Initialises an empty string.
 *
 * `init_cap` is a byte capacity hint, 0 falls
 * back to the vector's default capacity (24).
 */
bool str_init(string *str_uninit, vector_allocator_t allocator,
              size_t init_cap) {
  if (unlikely(!__str_init_internal(str_uninit, allocator, init_cap))) {
    return (false);
  }

  __str_set_len_unchecked(str_uninit, 0);
  return (true);
}

/* Deep copy.
 *
 * Uses `this`'s allocator and mirrors `this`'s capacity.
 */
bool str_clone(const string *this, string *str_uninit) {
  size_t src_cap = vector_capacity(&this->_vec);
  size_t src_len = vector_length(&this->_vec);

  if (unlikely(
          !__str_init_internal(str_uninit, this->_vec._allocator, src_cap))) {
    return (false);
  }

  if (src_len > 0) {
    __str_append_bytes_unchecked(str_uninit, __str_ptr(this), src_len);
  } else {
    __str_set_len_unchecked(str_uninit, 0);
  }

  return (true);
}

/* Deep copy of the half open slice [start, end).
 *
 * Note: negative indices are resolved as `len + idx`.
 * Out of range inputs clamp.
 * Weird ranges returns an empty string.
 * Only failure is OOM.
 */
bool str_clone_slice(const string *this, string *str_uninit, int64_t start,
                     int64_t end) {
  size_t len = vector_length(&this->_vec);
  size_t s = __str_clamp_index((ssize_t)start, len);
  size_t e = __str_clamp_index((ssize_t)end, len);
  if (e < s) {
    e = s;
  }

  size_t n = e - s;
  size_t cap_hint;

  if (unlikely(!__str_add_size(n, 1, &cap_hint))) {
    return (false);
  }

  if (unlikely(
          !__str_init_internal(str_uninit, this->_vec._allocator, cap_hint))) {
    return (false);
  }

  if (n > 0) {
    __str_append_bytes_unchecked(str_uninit, __str_ptr(this) + s, n);
  } else {
    __str_set_len_unchecked(str_uninit, 0);
  }

  return (true);
}

/* Copies up to `max_len` bytes from `cstr`.
 *
 * Bounded read, never inspects past max_len, even if
 * cstr is not NUL terminated within that window.
 * Truncation is silent.
 */
bool str_from_cstr(string *str_uninit, vector_allocator_t allocator,
                   size_t max_len, const unsigned char *cstr) {
  size_t n = (cstr == NULL) ? 0 : __str_strnlen(cstr, max_len);

  size_t cap_hint;
  if (unlikely(!__str_add_size(n, 1, &cap_hint))) {
    return (false);
  }

  if (unlikely(!__str_init_internal(str_uninit, allocator, cap_hint))) {
    return (false);
  }

  if (n > 0) {
    __str_append_bytes_unchecked(str_uninit, cstr, n);
  } else {
    __str_set_len_unchecked(str_uninit, 0);
  }

  return (true);
}

/* Adopts an existing buffer.
 *
 * Note: caller guarantees `buf` was allocated by the same allocator, `capacity` is
 * the allocation size, len <= capacity - 1 and buf[len] == '\0'.
 */
void str_from_raw_parts(string *str_uninit, vector_allocator_t allocator,
                        unsigned char *buf, size_t len, size_t capacity) {
  vector_from_raw_parts(&str_uninit->_vec, allocator, buf,
                        sizeof(unsigned char), len, capacity, NULL);
}

/* Initialises from a printf-style format, truncated to `max_len` bytes.
 */
bool str_from_format(string *str_uninit, vector_allocator_t allocator,
                     size_t max_len, const char *format, ...) {
  va_list args;
  va_start(args, format);

  size_t want;
  if (unlikely(!__str_format_probe(max_len, format, args, &want))) {
    va_end(args);
    return (false);
  }

  size_t cap_hint;
  if (unlikely(!__str_add_size(want, 1, &cap_hint))) {
    va_end(args);
    return (false);
  }

  if (unlikely(!__str_init_internal(str_uninit, allocator, cap_hint))) {
    va_end(args);
    return (false);
  }

  (void)vsnprintf((char *)__str_ptr(str_uninit), want + 1, format, args);
  va_end(args);

  __vector_set_length_internal(&str_uninit->_vec, want);
  return (true);
}

/* Releases the buffer and zeros `this`.
 */
void str_deinit(string *this) {
  vector_deinit(&this->_vec);
  (void)memset(this, 0, sizeof(*this));
}

/* Returns length in bytes, excluding the NUL terminator.
 */
size_t str_length(const string *this) { return (vector_length(&this->_vec)); }

/* Return true if length is 0.
 */
bool str_is_empty(const string *this) { return (vector_is_empty(&this->_vec)); }

/* Resets length to 0 and writes the NUL.
 *
 * Capacity unchanged.
 */
void str_clear(string *this) { __str_set_len_unchecked(this, 0); }

/* Pointer to the byte at `position`.
 *
 * Note: position in [-len, len-1] -> byte.
 * Position == len -> trailing NUL
 * otherwise NULL.
 *
 * IMPORTANT: The pointer returns by this is invalidated by
 * any growing operation performed later.
 */
unsigned char *str_index_to_ptr(const string *this, ssize_t position) {
  ssize_t signed_len = (ssize_t)vector_length(&this->_vec);
  ssize_t p = position;

  if (p < 0) {
    p += signed_len;
    if (p < 0) {
      return (NULL);
    }
  } else if (p > signed_len) {
    return (NULL);
  }

  return (__str_ptr(this) + (size_t)p);
}

/* Appends one byte to the end of the string.
 */
bool str_push_char(string *this, unsigned char c) {
  if (unlikely(!__str_reserve(this, 1))) {
    return (false);
  }
  __str_append_bytes_unchecked(this, &c, 1);
  return (true);
}

/* Appends up to `max_len` bytes from `cstr`.
 *
 * Bounded read.
 */
bool str_push_cstr(string *this, size_t max_len, const unsigned char *cstr) {
  if (cstr == NULL) {
    return (true);
  }

  size_t n = __str_strnlen(cstr, max_len);
  if (n == 0) {
    return (true);
  }

  if (unlikely(!__str_reserve(this, n))) {
    return (false);
  }

  __str_append_bytes_unchecked(this, cstr, n);
  return (true);
}

/* Appends `other` into `this`.
 *
 * Safe when other == this.
 */
bool str_append(string *this, const string *other) {
  size_t n = vector_length(&other->_vec);
  if (n == 0) {
    return (true);
  }

  if (unlikely(!__str_reserve(this, n))) {
    return (false);
  }

  size_t at = vector_length(&this->_vec);
  vector_copy_contiguous_within_inner_unchecked(&this->_vec, at,
                                                __str_ptr(other), n);
  __str_nul_terminate(this);
  return (true);
}

/* Appends formatted output, truncated to `max_len` bytes.
 */
bool str_append_format(string *this, size_t max_len, const char *format, ...) {
  va_list args;
  va_start(args, format);

  size_t want;
  if (unlikely(!__str_format_probe(max_len, format, args, &want))) {
    va_end(args);
    return (false);
  }

  if (want == 0) {
    va_end(args);
    return (true);
  }

  if (unlikely(!__str_reserve(this, want))) {
    va_end(args);
    return (false);
  }

  size_t at = vector_length(&this->_vec);
  (void)vsnprintf((char *)vector_index_to_ptr_unchecked(&this->_vec, at),
                  want + 1, format, args);
  va_end(args);

  __vector_set_length_internal(&this->_vec, at + want);
  return (true);
}

/* Removes the last byte, returned as int in [0, 255], or -1 on empty.
 */
int str_pop_char(string *this) {
  size_t len = vector_length(&this->_vec);
  if (len == 0) {
    return (-1);
  }

  int c = (int)__str_ptr(this)[len - 1];
  __str_set_len_unchecked(this, len - 1);

  return (c);
}

/* Inserts `c` at position 0.
 */
bool str_pushf_char(string *this, unsigned char c) {
  if (unlikely(!__str_reserve(this, 1))) {
    return (false);
  }

  __str_insert_bytes_unchecked(this, 0, &c, 1);
  return (true);
}

/* Removes the byte at position 0, returned as int in [0, 255], or -1 on
 * empty.
 */
int str_popf_char(string *this) {
  size_t len = vector_length(&this->_vec);
  if (len == 0) {
    return (-1);
  }

  int c = (int)__str_ptr(this)[0];
  __str_remove_bytes_unchecked(this, 0, 1);
  return (c);
}

/* Inserts before `position`.
 *
 * position == len behaves like append.
 * position > len returns false.
 */
bool str_insert_char(string *this, size_t position, unsigned char c) {
  if (unlikely(position > vector_length(&this->_vec))) {
    return (false);
  }

  if (unlikely(!__str_reserve(this, 1))) {
    return (false);
  }

  __str_insert_bytes_unchecked(this, position, &c, 1);
  return (true);
}

/* Inserts before `position`.
 *
 * position == len behaves like append.
 * position > len returns false.
 */
bool str_insert_cstr(string *this, size_t position, size_t max_len,
                     const unsigned char *cstr) {
  if (unlikely(position > vector_length(&this->_vec))) {
    return (false);
  }

  if (cstr == NULL) {
    return (true);
  }

  size_t n = __str_strnlen(cstr, max_len);
  if (n == 0) {
    return (true);
  }

  if (unlikely(!__str_reserve(this, n))) {
    return (false);
  }

  __str_insert_bytes_unchecked(this, position, cstr, n);
  return (true);
}

/* Inserts before `position`.
 *
 * position == len behaves like append.
 * position > len returns false.
 */
bool str_insert_other(string *this, size_t position, const string *other) {
  if (unlikely(position > vector_length(&this->_vec))) {
    return (false);
  }

  if (unlikely(this == other)) {
    string tmp;
    if (unlikely(!str_clone(other, &tmp))) {
      return (false);
    }

    bool ok = str_insert_other(this, position, &tmp);
    str_deinit(&tmp);
    return (ok);
  }

  size_t n = vector_length(&other->_vec);
  if (n == 0) {
    return (true);
  }

  if (unlikely(!__str_reserve(this, n))) {
    return (false);
  }

  __str_insert_bytes_unchecked(this, position, __str_ptr(other), n);
  return (true);
}

/* Inserts before `position`.
 *
 * position == len behaves like append.
 * position > len returns false.
 */
bool str_insert_format(string *this, size_t position, size_t max_len,
                       const char *format, ...) {
  if (unlikely(position > vector_length(&this->_vec))) {
    return (false);
  }

  va_list args;
  va_start(args, format);

  size_t want;
  if (unlikely(!__str_format_probe(max_len, format, args, &want))) {
    va_end(args);
    return (false);
  }

  if (want == 0) {
    va_end(args);
    return (true);
  }

  size_t tmp_size;
  if (unlikely(!__str_add_size(want, 1, &tmp_size))) {
    va_end(args);
    return (false);
  }

  unsigned char *tmp = this->_vec._allocator.alloc(tmp_size);
  if (unlikely(tmp == NULL)) {
    va_end(args);
    return (false);
  }

  (void)vsnprintf((char *)tmp, tmp_size, format, args);
  va_end(args);

  if (unlikely(!__str_reserve(this, want))) {
    this->_vec._allocator.release(tmp);
    return (false);
  }

  __str_insert_bytes_unchecked(this, position, tmp, want);
  this->_vec._allocator.release(tmp);
  return (true);
}

/* Removes one byte at `position`.
 *
 * Out of range is a no-op.
 */
void str_remove(string *this, size_t position) {
  if (position >= vector_length(&this->_vec)) {
    return;
  }

  __str_remove_bytes_unchecked(this, position, 1);
}

/* Removes the half-open range [start, end).
 *
 * end > len clamps; end <= start is a no-op.
 */
void str_remove_range(string *this, size_t start, size_t end) {
  size_t len = vector_length(&this->_vec);
  if (start >= len || end <= start) {
    return;
  }

  if (end > len) {
    end = len;
  }

  __str_remove_bytes_unchecked(this, start, end - start);
}

/* Shrinks length to `len`.
 *
 * len >= current length is a no-op.
 * Capacity is not changed.
 */
void str_truncate(string *this, size_t len) {
  if (len >= vector_length(&this->_vec)) {
    return;
  }

  __str_set_len_unchecked(this, len);
}

/* Reallocates to a tight buffer (len + 1 bytes) if more than half the
 * capacity is unused.
 *
 * Returns false on OOM.
 * the string is unchanged.
 */
bool str_shrink_to_fit(string *this) {
  size_t target;
  if (unlikely(!__str_add_size(vector_length(&this->_vec), 1, &target))) {
    return (false);
  }

  size_t cap = vector_capacity(&this->_vec);
  if (cap == 0 || target >= cap / 2) {
    return (true);
  }

  unsigned char *new_buf = this->_vec._allocator.alloc(target);
  if (unlikely(new_buf == NULL)) {
    return (false);
  }

  size_t len = vector_length(&this->_vec);
  if (len > 0) {
    (void)memcpy(new_buf, __str_ptr(this), len);
  }
  new_buf[len] = '\0';

  this->_vec._allocator.release(__str_ptr(this));
  __vector_set_ptr_internal(&this->_vec, new_buf);
  __vector_set_capacity_internal(&this->_vec, target);
  return (true);
}

/* Transfers ownership of the buffer to the caller and zeros *this.
 *
 * The returned buffer is NUL terminated and must be released with the same
 * allocator used at construction.
 *
 * Note: call str_shrink_to_fit first if you want a tight allocation.
 */
unsigned char *str_leak_cstr(string *this) {
  unsigned char *buf = __str_ptr(this);
  (void)memset(this, 0, sizeof(*this));

  return (buf);
}