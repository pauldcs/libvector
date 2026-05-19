#include "vector.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* The string container owns a byte buffer through the vector
 * primitive. The buffer always contains at least one byte: a
 * trailing NUL stored at the last position of the underlying vector.
 * The vector's length is exactly one more than the
 * caller visible string length, and the underlying vector is never
 * empty. Every method in this file is responsible for
 * preserving that invariant.
 */

/* Writes a formatted string into `dst`, capped at `dst_size` bytes
 * (NUL included). The return is true on success and false on
 * encoding error. On success `*out_written` holds the number of
 * bytes written, NUL excluded.
 *
 * Preconditions: `dst` points to at least `dst_size` writable bytes,
 * `format` is a valid printf-style format string, `args` is a valid
 * va_list matching `format`, and `out_written` is non-NULL.
 */
__no_discard static inline bool __string_vsnprintf(char *dst, size_t dst_size,
                                                   const char *format,
                                                   va_list args,
                                                   size_t *out_written) {
  __bug_if_fail__(dst != NULL || dst_size == 0);
  __bug_if_fail__(format != NULL);
  __bug_if_fail__(out_written != NULL);

  int written;

  written = vsnprintf(dst, dst_size, format, args);
  if (unlikely(written < 0)) {
    return (false);
  }

  *out_written = (size_t)written;
  return (true);
}

/* Allocates `n` bytes through the same allocator the string was
 * constructed with.
 *
 * Preconditions: `this` is a valid initialised string and `out` is
 * non-NULL.
 * Returns: true on success and writes the new buffer to `*out`. On
 * failure `*out` is unspecified.
 */
__no_discard static inline bool __string_alloc(string *this, size_t n,
                                               unsigned char **out) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(out != NULL);

  *out = this->_vec._allocator.alloc(n);
  return (*out != NULL);
}

/* Returns a pointer to the first byte of the string's buffer. The
 * pointer-returning shape mirrors the underlying vector accessor:
 * this helper is used in inner loops where any indirection would
 * dominate the cost of the access. The buffer is non-empty by the
 * module's invariant, so the pointer is always valid.
 *
 * Preconditions: `this` is a valid initialised string.
 */
__pure1 static inline __attribute__((always_inline)) unsigned char *
__string_ptr(const string *this) {
  __bug_if_fail__(this != NULL);

  return ((unsigned char *)vector_first_to_ptr_unchecked(&this->_vec));
}

/* Returns the caller-visible string length. The underlying vector
 * carries one extra byte for the trailing NUL, which this helper
 * subtracts.
 *
 * Preconditions: `this` is a valid initialised string.
 */
__pure1 static inline __attribute__((always_inline)) size_t
__string_len(const string *this) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(vector_length(&this->_vec) >= 1);

  return (vector_length(&this->_vec) - 1);
}

/* Returns the position of the first NUL byte in `bytes[0..max)`, or
 * `max` if no NUL was found. Bounded so that `bytes` need not be
 * NUL-terminated within the window.
 *
 * Preconditions: `bytes` is readable for at least `max` bytes.
 */
__pure1 static inline __attribute__((always_inline)) size_t
__string_strnlen(const unsigned char *bytes, size_t max) {
  __bug_if_fail__(bytes != NULL || max == 0);

  size_t i;

  i = 0;
  while (i < max && bytes[i] != '\0') {
    ++i;
  }

  return (i);
}

/* Performs the addition `left + right` with overflow detection.
 *
 * Preconditions: `out` is non-NULL.
 * Returns: true and writes the sum to `*out` when the result fits in
 * `size_t`. On overflow `*out` is unspecified and false is returned.
 */
__no_discard static inline __attribute__((always_inline)) bool
__string_add_size(size_t left, size_t right, size_t *out) {
  __bug_if_fail__(out != NULL);

  if (unlikely(left > SIZE_MAX - right)) {
    return (false);
  }

  *out = left + right;
  return (true);
}

/* Ensures the buffer can hold `n` additional bytes beyond what is
 * currently in the vector. The trailing NUL is already counted in
 * the vector's length, so no extra byte needs to be requested here.
 *
 * Preconditions: `this` is a valid initialised string.
 * Returns: true on success. On failure the buffer is unchanged.
 */
__no_discard inline __attribute__((always_inline)) bool
__string_reserve(string *this, size_t n) {
  __bug_if_fail__(this != NULL);

  return (vector_adjust_cap_if_full(&this->_vec, n));
}

/* Converts a possibly-negative caller-supplied index into a clamped
 * non-negative offset in [0, len]. Negative indices are interpreted
 * as offsets from the end (`idx + len`). The signed `idx` parameter
 * is intentional: the public API contract supports negative indices
 * and a signed type is the only way to express them.
 *
 * Preconditions: none.
 */
__pure1 static inline __attribute__((always_inline)) size_t
__string_clamp_index(ssize_t idx, size_t len) {
  ssize_t signed_len;

  signed_len = (ssize_t)len;
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

/* Initialises the underlying vector with element size 1 and a byte
 * capacity hint of `init_cap`, then seeds it with a single NUL byte
 * so the module's invariant holds from the moment the function
 * returns.
 *
 * Preconditions: `this` points to writable storage of at least
 * `sizeof(string)` bytes.
 * Returns: true on success. On failure no buffer has been allocated.
 */
__no_discard static inline __attribute__((always_inline)) bool
__string_init(string *this, vector_allocator_t allocator, size_t init_cap) {
  __bug_if_fail__(this != NULL);

  const unsigned char nul = '\0';

  if (unlikely(!vector_init(&this->_vec, allocator, sizeof(unsigned char),
                            init_cap, NULL))) {
    return (false);
  }

  if (unlikely(!vector_push(&this->_vec, &nul))) {
    vector_deinit(&this->_vec);
    return (false);
  }

  return (true);
}

/* Probes how many bytes a printf-style format would produce when
 * rendered, capped at `max_len`. The probe operates on a copy of
 * `args`; the caller's `args` is unchanged and ready for the actual
 * write.
 *
 * Preconditions: `format` is a valid format string, `args` is a
 * valid matching va_list, and `out_want` is non-NULL.
 * Returns: true on success and writes the (possibly clamped) byte
 * count to `*out_want`. On encoding error `*out_want` is unspecified.
 */
__no_discard static inline bool __string_format_probe(size_t max_len,
                                                      const char *format,
                                                      va_list args,
                                                      size_t *out_want) {
  __bug_if_fail__(format != NULL);
  __bug_if_fail__(out_want != NULL);

  va_list probe;
  size_t want;
  bool ok;

  va_copy(probe, args);
  ok = __string_vsnprintf(NULL, 0, format, probe, &want);
  va_end(probe);

  if (unlikely(!ok)) {
    return (false);
  }

  if (want > max_len) {
    want = max_len;
  }

  *out_want = want;
  return (true);
}

/* Inserts `n` bytes from `src` before string-position `position`,
 * shifting the existing tail (including the trailing NUL) to the
 * right. The NUL therefore remains the last byte of the vector
 * without any further work.
 *
 * Preconditions: `this` is a valid initialised string with at least
 * `n` free bytes of capacity beyond its current vector length, and
 * `position` is in [0, length].
 */
static inline __attribute__((always_inline)) void
__string_insert_bytes_unchecked(string *this, size_t position, const void *src,
                                size_t n) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(src != NULL || n == 0);
  __bug_if_fail__(position <= __string_len(this));

  vector_copy_contiguous_within_inner_unchecked(&this->_vec, position, src, n);
}

/* Removes `n` bytes starting at string-position `start`. The
 * underlying vector shift naturally pulls the trailing NUL down with
 * the rest of the tail, so the invariant is preserved without any
 * additional write.
 *
 * Preconditions: `this` is a valid initialised string and
 * `start + n <= length`.
 */
static inline __attribute__((always_inline)) void
__string_remove_bytes_unchecked(string *this, size_t start, size_t n) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(start + n <= __string_len(this));

  vector_leak_range_unchecked(&this->_vec, start, n);
}

/* Initialises an empty string in caller-provided storage. `init_cap`
 * is a byte capacity hint; 0 falls back to the underlying container's
 * default.
 *
 * Preconditions: `string_uninit` points to writable storage of at least
 * `sizeof(string)` bytes and `allocator` is a valid allocator pair.
 *
 * On success the string is empty, NUL-terminated, and ready for use.
 * On failure no buffer has been allocated and the caller has no
 * cleanup to perform.
 */
__no_discard bool string_init(string *string_uninit,
                              vector_allocator_t allocator, size_t init_cap) {
  __bug_if_fail__(string_uninit != NULL);
  __bug_if_fail__(allocator.alloc != NULL);
  __bug_if_fail__(allocator.release != NULL);

  return (__string_init(string_uninit, allocator, init_cap));
}

/* Initialises `string_uninit` as a deep copy of `this`. The new string
 * uses the source's allocator and mirrors its capacity.
 *
 * Preconditions: `this` is a valid initialised string and
 * `string_uninit` points to writable storage of at least `sizeof(string)`
 * bytes.
 * Returns: true on success. On failure no buffer has been allocated
 * and the caller has no cleanup to perform.
 */
__no_discard bool string_clone(const string *this, string *string_uninit) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(string_uninit != NULL);

  const size_t src_cap = vector_capacity(&this->_vec);
  const size_t src_len = __string_len(this);

  if (unlikely(!__string_init(string_uninit, this->_vec._allocator, src_cap))) {
    return (false);
  }

  if (src_len > 0) {
    if (unlikely(!__string_reserve(string_uninit, src_len))) {
      string_deinit(string_uninit);
      return (false);
    }
    __string_insert_bytes_unchecked(string_uninit, 0, __string_ptr(this),
                                    src_len);
  }

  return (true);
}

/* Initialises `string_uninit` as a deep copy of the half-open slice
 * `[start, end)` of `this`. Negative indices are interpreted as
 * offsets from the end. Out-of-range inputs are clamped to the
 * string's bounds, and an empty or inverted slice produces an empty
 * result.
 *
 * Preconditions: `this` is a valid initialised string and
 * `string_uninit` points to writable storage of at least
 * `sizeof(string)` bytes.
 * Returns: true on success. On failure no buffer has been allocated
 * and the caller has no cleanup to perform.
 */
__no_discard bool string_clone_slice(const string *this, string *string_uninit,
                                     int64_t start, int64_t end) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(string_uninit != NULL);

  const size_t len = __string_len(this);
  size_t start_clamped;
  size_t end_clamped;
  size_t n;
  size_t cap_hint;

  start_clamped = __string_clamp_index((ssize_t)start, len);
  end_clamped = __string_clamp_index((ssize_t)end, len);
  if (end_clamped < start_clamped) {
    end_clamped = start_clamped;
  }

  n = end_clamped - start_clamped;

  if (unlikely(!__string_add_size(n, 1, &cap_hint))) {
    return (false);
  }

  if (unlikely(
          !__string_init(string_uninit, this->_vec._allocator, cap_hint))) {
    return (false);
  }

  if (n > 0) {
    if (unlikely(!__string_reserve(string_uninit, n))) {
      string_deinit(string_uninit);
      return (false);
    }
    __string_insert_bytes_unchecked(string_uninit, 0,
                                    __string_ptr(this) + start_clamped, n);
  }

  return (true);
}

/* Initialises `string_uninit` with a copy of up to `max_len` bytes
 * read from `cstr`. The read is bounded: this function never
 * inspects past `max_len`, even if `cstr` is not NUL-terminated
 * within that window. Truncation is silent.
 *
 * Preconditions: `string_uninit` points to writable storage of at least
 * `sizeof(string)` bytes, `allocator` is a valid allocator pair, and
 * either `cstr` is readable for at least `max_len` bytes or `cstr`
 * is NULL (treated as an empty input).
 * Returns: true on success. On failure no buffer has been allocated
 * and the caller has no cleanup to perform.
 */
__no_discard bool string_from_cstr(string *string_uninit,
                                   vector_allocator_t allocator, size_t max_len,
                                   const unsigned char *cstr) {
  __bug_if_fail__(string_uninit != NULL);
  __bug_if_fail__(allocator.alloc != NULL);
  __bug_if_fail__(allocator.release != NULL);

  size_t n;
  size_t cap_hint;

  n = (cstr == NULL) ? 0 : __string_strnlen(cstr, max_len);

  if (unlikely(!__string_add_size(n, 1, &cap_hint))) {
    return (false);
  }

  if (unlikely(!__string_init(string_uninit, allocator, cap_hint))) {
    return (false);
  }

  if (n > 0) {
    if (unlikely(!__string_reserve(string_uninit, n))) {
      string_deinit(string_uninit);
      return (false);
    }
    __string_insert_bytes_unchecked(string_uninit, 0, cstr, n);
  }

  return (true);
}

/* Adopts an existing buffer as the storage of `string_uninit`. No copy
 * is made; the string takes ownership of `buf`. The buffer must
 * already contain a NUL at `buf[len]`, which becomes the string's
 * trailing NUL inside the underlying vector.
 *
 * Preconditions: `string_uninit` points to writable storage of at least
 * `sizeof(string)` bytes, `buf` was allocated by `allocator`,
 * `capacity` is the size of that allocation in bytes, `len + 1 <=
 * capacity`, and `buf[len] == '\0'`.
 */
void string_from_raw_parts(string *string_uninit, vector_allocator_t allocator,
                           unsigned char *buf, size_t len, size_t capacity) {
  __bug_if_fail__(string_uninit != NULL);
  __bug_if_fail__(buf != NULL);
  __bug_if_fail__(allocator.alloc != NULL);
  __bug_if_fail__(allocator.release != NULL);
  __bug_if_fail__(len + 1 <= capacity);
  __bug_if_fail__(buf[len] == '\0');

  vector_from_raw_parts(&string_uninit->_vec, allocator, buf,
                        sizeof(unsigned char), len + 1, capacity, NULL);
}

/* Adopts an existing buffer as the storage of `string_uninit`. No copy
 * is made, the string takes ownership of `buf`. The buffer must
 * already contain a NUL at `buf[len]`, which becomes the string's
 * trailing NUL inside the underlying vector.
 *
 * Preconditions: `string_uninit` points to writable storage of at least
 * `sizeof(string)` bytes, `capacity` is the size of the static string,
 * `len + 1 <= capacity`, and `buf[len] == '\0'`.
 *
 * # IMPORTANT:
 * this string constructor is intended to be used with static string that
 * cannot grow, thus, every pointer within it are stable across the lifetime of
 * the string. It does not have an allocator, any method that attempts to grow
 * it will fail. It is advised to only use `*within_inner` methods.
 */
void string_from_raw_static_parts(string *string_uninit, unsigned char *buf,
                                  size_t len, size_t capacity) {
  __bug_if_fail__(string_uninit != NULL);
  __bug_if_fail__(buf != NULL);
  __bug_if_fail__(len + 1 <= capacity);
  __bug_if_fail__(buf[len] == '\0');

  vector_from_raw_static_parts(&string_uninit->_vec, buf, sizeof(unsigned char),
                               len + 1, capacity, NULL);
}

/* Initialises `string_uninit` with the rendering of a printf-style
 * format, truncated to at most `max_len` bytes (NUL excluded).
 *
 * Preconditions: `string_uninit` points to writable storage of at least
 * `sizeof(string)` bytes, `allocator` is a valid allocator pair, and
 * `format` is a valid format string with arguments to match.
 * Returns: true on success. On failure no buffer has been allocated
 * and the caller has no cleanup to perform.
 */
__no_discard bool string_from_format(string *string_uninit,
                                     vector_allocator_t allocator,
                                     size_t max_len, const char *format, ...) {
  __bug_if_fail__(string_uninit != NULL);
  __bug_if_fail__(allocator.alloc != NULL);
  __bug_if_fail__(allocator.release != NULL);
  __bug_if_fail__(format != NULL);

  va_list args;
  size_t want;
  size_t cap_hint;
  size_t written;

  va_start(args, format);

  if (unlikely(!__string_format_probe(max_len, format, args, &want)) ||
      unlikely(!__string_add_size(want, 1, &cap_hint)) ||
      unlikely(!__string_init(string_uninit, allocator, cap_hint))) {
    va_end(args);
    return (false);
  }

  if (want > 0) {
    if (unlikely(!__string_reserve(string_uninit, want))) {
      va_end(args);
      string_deinit(string_uninit);
      return (false);
    }
    (void)__string_vsnprintf((char *)__string_ptr(string_uninit), want + 1,
                             format, args, &written);
    __vector_set_length_internal(&string_uninit->_vec, want + 1);
    __string_ptr(string_uninit)[want] = '\0';
  }
  va_end(args);

  return (true);
}

/* Releases the internal buffer and zeroes the string descriptor.
 * After this call the string is uninitialised and may only be reused
 * by passing it to `string_init` again.
 *
 * Preconditions: `this` is a valid initialised string.
 */
void string_deinit(string *this) {
  __bug_if_fail__(this != NULL);

  vector_deinit(&this->_vec);
  __memset(this, 0, sizeof(*this));
}

/* Returns the number of bytes in the string, NUL excluded.
 *
 * Preconditions: `this` is a valid initialised string.
 */
__pure1 size_t string_length(const string *this) {
  __bug_if_fail__(this != NULL);

  return (__string_len(this));
}

/* Returns true when the string contains zero bytes. This is a
 * comparison against the module's empty-state encoding (vector
 * length one, holding only the trailing NUL); the underlying
 * vector is never empty.
 *
 * Preconditions: `this` is a valid initialised string.
 */
__pure1 bool string_is_empty(const string *this) {
  __bug_if_fail__(this != NULL);

  return (__string_len(this) == 0);
}

/* Sets the visible length to zero. Capacity is preserved and the
 * underlying vector is left holding only the trailing NUL.
 *
 * Preconditions: `this` is a valid initialised string.
 */
void string_clear(string *this) {
  __bug_if_fail__(this != NULL);

  __vector_set_length_internal(&this->_vec, 1);
  __string_ptr(this)[0] = '\0';
}

/* Returns a pointer to the byte at `position`. Negative indices are
 * interpreted as offsets from the end (`position + length`).
 * `position == length` returns the trailing NUL. Out-of-range
 * inputs return NULL. The pointer-returning shape mirrors the
 * underlying vector accessor; the returned pointer is invalidated
 * by any subsequent operation that may grow the buffer.
 *
 * Preconditions: `this` is a valid initialised string. The signed
 * `position` is intentional: negative indices are part of the API
 * contract and a signed type is the only way to express them.
 */
__pure1 unsigned char *string_index_to_ptr(const string *this,
                                           ssize_t position) {
  __bug_if_fail__(this != NULL);

  ssize_t signed_len;
  ssize_t clamped_position;

  signed_len = (ssize_t)__string_len(this);
  clamped_position = position;

  if (clamped_position < 0) {
    clamped_position += signed_len;
    if (clamped_position < 0) {
      return (NULL);
    }
  } else if (clamped_position > signed_len) {
    return (NULL);
  }

  return (__string_ptr(this) + (size_t)clamped_position);
}

/* Appends one byte to the end of the string.
 *
 * Preconditions: `this` is a valid initialised string.
 * Returns: true on success. On failure the string is unchanged.
 */
__no_discard bool string_push_char(string *this, unsigned char byte) {
  __bug_if_fail__(this != NULL);

  if (unlikely(!__string_reserve(this, 1))) {
    return (false);
  }

  __string_insert_bytes_unchecked(this, __string_len(this), &byte, 1);
  return (true);
}

/* Appends up to `max_len` bytes from `cstr`. The read is bounded:
 * this function never inspects past `max_len`, even if `cstr` is not
 * NUL-terminated within that window.
 *
 * Preconditions: `this` is a valid initialised string and either
 * `cstr` is readable for at least `max_len` bytes or `cstr` is NULL.
 * Returns: true on success. On failure the string is unchanged.
 */
__no_discard bool string_push_cstr(string *this, size_t max_len,
                                   const unsigned char *cstr) {
  __bug_if_fail__(this != NULL);

  size_t n;

  if (cstr == NULL) {
    return (true);
  }

  n = __string_strnlen(cstr, max_len);
  if (n == 0) {
    return (true);
  }

  if (unlikely(!__string_reserve(this, n))) {
    return (false);
  }

  __string_insert_bytes_unchecked(this, __string_len(this), cstr, n);
  return (true);
}

/* Appends the contents of `other` to the end of `this`. Safe when
 * `other == this`.
 *
 * Preconditions: `this` and `other` are valid initialised strings.
 * Returns: true on success. On failure `this` is unchanged.
 */
__no_discard bool string_append(string *this, const string *other) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(other != NULL);

  size_t n;

  n = __string_len(other);
  if (n == 0) {
    return (true);
  }

  if (unlikely(!__string_reserve(this, n))) {
    return (false);
  }

  __string_insert_bytes_unchecked(this, __string_len(this), __string_ptr(other),
                                  n);
  return (true);
}

/* Appends the rendering of a printf-style format to the end of
 * `this`, truncated to at most `max_len` bytes.
 *
 * Preconditions: `this` is a valid initialised string and `format`
 * is a valid format string with arguments to match.
 * Returns: true on success. On failure `this` is unchanged.
 */
__no_discard bool string_append_format(string *this, size_t max_len,
                                       const char *format, ...) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(format != NULL);

  va_list args;
  size_t want;
  size_t at;
  size_t written;

  va_start(args, format);

  if (unlikely(!__string_format_probe(max_len, format, args, &want))) {
    va_end(args);
    return (false);
  }

  if (want == 0) {
    va_end(args);
    return (true);
  }

  if (unlikely(!__string_reserve(this, want))) {
    va_end(args);
    return (false);
  }

  at = __string_len(this);
  (void)__string_vsnprintf((char *)__string_ptr(this) + at, want + 1, format,
                           args, &written);
  va_end(args);

  __vector_set_length_internal(&this->_vec, at + want + 1);
  __string_ptr(this)[at + want] = '\0';
  return (true);
}

/* Removes the last byte and returns it as an int in [0, 255]. An
 * empty string yields -1.
 *
 * Preconditions: `this` is a valid initialised string.
 */
int string_pop_char(string *this) {
  __bug_if_fail__(this != NULL);

  size_t len;
  int byte;

  len = __string_len(this);
  if (len == 0) {
    return (-1);
  }

  byte = (int)__string_ptr(this)[len - 1];
  __string_remove_bytes_unchecked(this, len - 1, 1);

  return (byte);
}

/* Inserts one byte at position 0.
 *
 * Preconditions: `this` is a valid initialised string.
 * Returns: true on success. On failure the string is unchanged.
 */
__no_discard bool string_pushf_char(string *this, unsigned char byte) {
  __bug_if_fail__(this != NULL);

  if (unlikely(!__string_reserve(this, 1))) {
    return (false);
  }

  __string_insert_bytes_unchecked(this, 0, &byte, 1);
  return (true);
}

/* Removes the byte at position 0 and returns it as an int in
 * [0, 255]. An empty string yields -1.
 *
 * Preconditions: `this` is a valid initialised string.
 */
int string_popf_char(string *this) {
  __bug_if_fail__(this != NULL);

  size_t len;
  int byte;

  len = __string_len(this);
  if (len == 0) {
    return (-1);
  }

  byte = (int)__string_ptr(this)[0];
  __string_remove_bytes_unchecked(this, 0, 1);
  return (byte);
}

/* Inserts a byte before `position`. `position == length` behaves as
 * an append; `position > length` is rejected.
 *
 * Preconditions: `this` is a valid initialised string.
 * Returns: true on success. On failure the string is unchanged.
 */
__no_discard bool string_insert_char(string *this, size_t position,
                                     unsigned char byte) {
  __bug_if_fail__(this != NULL);

  if (unlikely(position > __string_len(this))) {
    return (false);
  }

  if (unlikely(!__string_reserve(this, 1))) {
    return (false);
  }

  __string_insert_bytes_unchecked(this, position, &byte, 1);
  return (true);
}

/* Inserts up to `max_len` bytes from `cstr` before `position`.
 * `position == length` behaves as an append; `position > length` is
 * rejected. The read is bounded.
 *
 * Preconditions: `this` is a valid initialised string and either
 * `cstr` is readable for at least `max_len` bytes or `cstr` is NULL.
 * Returns: true on success. On failure the string is unchanged.
 */
__no_discard bool string_insert_cstr(string *this, size_t position,
                                     size_t max_len,
                                     const unsigned char *cstr) {
  __bug_if_fail__(this != NULL);

  size_t n;

  if (unlikely(position > __string_len(this))) {
    return (false);
  }

  if (cstr == NULL) {
    return (true);
  }

  n = __string_strnlen(cstr, max_len);
  if (n == 0) {
    return (true);
  }

  if (unlikely(!__string_reserve(this, n))) {
    return (false);
  }

  __string_insert_bytes_unchecked(this, position, cstr, n);
  return (true);
}

/* Inserts the contents of `other` before `position` in `this`.
 * `position == length` behaves as an append; `position > length` is
 * rejected. Safe when `other == this`: a temporary copy is taken to
 * avoid overlapping reads and writes.
 *
 * Preconditions: `this` and `other` are valid initialised strings.
 * Returns: true on success. On failure `this` is unchanged.
 */
__no_discard bool string_insert_other(string *this, size_t position,
                                      const string *other) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(other != NULL);

  string tmp;
  size_t n;

  if (unlikely(position > __string_len(this))) {
    return (false);
  }

  if (unlikely(this == other)) {
    if (!string_clone(other, &tmp)) {
      goto prison;
    }
    if (!string_insert_other(this, position, &tmp)) {
      string_deinit(&tmp);
      goto prison;
    }
    string_deinit(&tmp);
    goto beach;
  }

  n = __string_len(other);
  if (n == 0) {
    goto beach;
  }

  if (unlikely(!__string_reserve(this, n))) {
    goto prison;
  }

  __string_insert_bytes_unchecked(this, position, __string_ptr(other), n);
  goto beach;

prison:
  return (false);
beach:
  return (true);
}

/* Inserts the rendering of a printf-style format before `position`
 * in `this`, truncated to at most `max_len` bytes. `position ==
 * length` behaves as an append; `position > length` is rejected.
 *
 * Preconditions: `this` is a valid initialised string and `format`
 * is a valid format string with arguments to match.
 * Returns: true on success. On failure `this` is unchanged.
 */
__no_discard bool string_insert_format(string *this, size_t position,
                                       size_t max_len, const char *format,
                                       ...) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(format != NULL);

  va_list args;
  size_t want;
  size_t tmp_size;
  size_t written;
  unsigned char *tmp;

  if (unlikely(position > __string_len(this))) {
    return (false);
  }

  va_start(args, format);

  if (unlikely(!__string_format_probe(max_len, format, args, &want))) {
    va_end(args);
    return (false);
  }

  if (want == 0) {
    va_end(args);
    return (true);
  }

  if (unlikely(!__string_add_size(want, 1, &tmp_size))) {
    va_end(args);
    return (false);
  }

  if (unlikely(!__string_alloc(this, tmp_size, &tmp))) {
    va_end(args);
    return (false);
  }

  (void)__string_vsnprintf((char *)tmp, tmp_size, format, args, &written);
  va_end(args);

  if (unlikely(!__string_reserve(this, want))) {
    this->_vec._allocator.release(tmp);
    goto prison;
  }

  __string_insert_bytes_unchecked(this, position, tmp, want);
  this->_vec._allocator.release(tmp);
  goto beach;

prison:
  return (false);
beach:
  return (true);
}

/* Removes one byte at `position`. An out-of-range `position` is a
 * no-op.
 *
 * Preconditions: `this` is a valid initialised string.
 */
void string_remove(string *this, size_t position) {
  __bug_if_fail__(this != NULL);

  if (position >= __string_len(this)) {
    return;
  }

  __string_remove_bytes_unchecked(this, position, 1);
}

/* Removes the half-open range `[start, end)`. `end > length` is
 * clamped; `end <= start` is a no-op.
 *
 * Preconditions: `this` is a valid initialised string.
 */
void string_remove_range(string *this, size_t start, size_t end) {
  __bug_if_fail__(this != NULL);

  const size_t len = __string_len(this);

  if (start >= len || end <= start) {
    return;
  }

  if (end > len) {
    end = len;
  }

  __string_remove_bytes_unchecked(this, start, end - start);
}

/* Shrinks the visible length to `len`. `len >= length` is a no-op.
 * Capacity is preserved.
 *
 * Preconditions: `this` is a valid initialised string.
 */
void string_truncate(string *this, size_t len) {
  __bug_if_fail__(this != NULL);

  const size_t cur = __string_len(this);

  if (len >= cur) {
    return;
  }

  __string_remove_bytes_unchecked(this, len, cur - len);
}

/* Reallocates to a tight buffer (`length + 1` bytes) if more than
 * half of the current capacity is unused.
 *
 * Preconditions: `this` is a valid initialised string.
 * Returns: true on success or when no shrink was necessary. On
 * allocation failure the string is unchanged and false is returned.
 */
__no_discard bool string_shrink_to_fit(string *this) {
  __bug_if_fail__(this != NULL);

  size_t target;
  size_t cap;
  size_t vlen;
  unsigned char *new_buf;

  vlen = vector_length(&this->_vec);

  if (unlikely(!__string_add_size(__string_len(this), 1, &target))) {
    return (false);
  }

  cap = vector_capacity(&this->_vec);
  if (cap == 0 || target >= cap / 2) {
    return (true);
  }

  if (unlikely(!__string_alloc(this, target, &new_buf))) {
    return (false);
  }

  __memcpy(new_buf, __string_ptr(this), vlen);

  this->_vec._allocator.release(__string_ptr(this));
  __vector_set_ptr_internal(&this->_vec, new_buf);
  __vector_set_capacity_internal(&this->_vec, target);
  return (true);
}

/* Transfers ownership of the internal buffer to the caller and
 * zeroes the string descriptor. The returned buffer is
 * NUL-terminated (the module's invariant places that NUL inside the
 * vector itself) and must later be released through the same
 * allocator the string was constructed with. Call
 * `string_shrink_to_fit` beforehand to obtain a tight allocation.
 *
 * Preconditions: `this` is a valid initialised string and `out_buf`
 * is non-NULL.
 *
 * Postconditions: `*out_buf` points to the buffer that previously
 * backed `this`; `this` has been zeroed and may only be reused by
 * passing it to `string_init` again. This function cannot fail.
 */
void string_leak_cstr(string *this, unsigned char **out_buf) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(out_buf != NULL);

  *out_buf = __string_ptr(this);
  __memset(this, 0, sizeof(*this));
}