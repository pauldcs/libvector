#include "vector.h"
#include <stdlib.h>

/* `malloc` used internally with `vector_from_raw_parts_static`.
 *
 * This will always fail, as a static buffer is not supposed to grow.
 */
void *__static_noop_malloc(size_t __size) {
  (void)__size;
  return (false);
}

/* `free` used internally with `vector_from_raw_parts_static`.
 */
void __static_noop_free(void *__ptr) { (void)__ptr; }

/* Sets the vector's stored length without touching the underlying buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline __attribute__((always_inline)) void
__vector_set_length_internal(vector *this, size_t len) {
  __bug_if_fail__(this != NULL);

  this->_len = len;
}

/* Sets the vector's stored element size without touching the underlying
 * buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline __attribute__((always_inline)) void
__vector_set_elem_size_internal(vector *this, size_t elem_size) {
  __bug_if_fail__(this != NULL);

  this->_elem_size = elem_size;
}

/* Sets the vector's stored capacity without touching the underlying
 * buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline __attribute__((always_inline)) void
__vector_set_capacity_internal(vector *this, size_t cap) {
  __bug_if_fail__(this != NULL);

  this->_capacity = cap;
}

/* Sets the vector's stored buffer pointer without freeing the previous
 * one.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline __attribute__((always_inline)) void
__vector_set_ptr_internal(vector *this, void *ptr) {
  __bug_if_fail__(this != NULL);

  this->_ptr = ptr;
}

/* Sets the vector's destructor function pointer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline __attribute__((always_inline)) void
__vector_set_destructor_internal(vector *this, void (*destructor)(void *)) {
  __bug_if_fail__(this != NULL);

  this->_destructor = destructor;
}

/* Allocates the vector's initial buffer using the configured allocator
 * and stores the resulting pointer.
 *
 * Preconditions: `this` is a valid vector with its allocator set, and
 * the buffer has not yet been allocated.
 * Returns: true on success, false on allocation failure.
 */
__no_discard bool __vector_alloc(vector *this, size_t init_size) {
  __bug_if_fail__(this != NULL);

  void *p;

  p = this->_allocator.alloc(init_size);
  if (unlikely(p == NULL)) {
    return (false);
  }

  __vector_set_ptr_internal(this, p);
  return (true);
}

/* Grows the buffer by allocating a new one, copying the live portion
 * across, releasing the old one, and updating the pointer and capacity.
 *
 * Preconditions: `this` is a valid initialised vector whose existing
 * contents fit in `new_size` bytes.
 * Returns: true on success. On failure the vector is unchanged.
 */
__no_discard bool __vector_realloc(vector *this, size_t new_size) {
  __bug_if_fail__(this != NULL);

  void *p;

  p = this->_allocator.alloc(new_size);
  if (unlikely(p == NULL)) {
    return (false);
  }

  (void)__memcpy(p, vector_first_to_ptr_unchecked(this),
                 this->_len * vector_elem_size(this));

  this->_allocator.release(vector_first_to_ptr_unchecked(this));

  __vector_set_ptr_internal(this, p);
  __vector_set_capacity_internal(this, new_size);

  return (true);
}

/* Initialises a vector in caller provided storage with enough capacity
 * to hold at least `init_cap` elements of `elem_size`. An
 * `init_cap` of 0 selects a default starting capacity.
 *
 * Preconditions: `uninit_vec` points to at least `sizeof(vector)` bytes
 * of writable storage. `elem_size` is non-zero. `allocator` is a valid
 * allocator pair.
 *
 * On success the vector is empty (length 0) and may be used with any
 * vector operation. On failure no buffer has been allocated and the
 * caller has no cleanup to perform.
 */
__no_discard bool vector_init(vector *uninit_vec, vector_allocator_t allocator,
                              size_t elem_size, size_t init_cap,
                              void (*destroy)(void *)) {
  __bug_if_fail__(uninit_vec != NULL);
  __bug_if_fail__(elem_size != 0);
  __bug_if_fail__(allocator.alloc != NULL);
  __bug_if_fail__(allocator.release != NULL);

  const size_t capacity = elem_size * (init_cap == 0 ? MIN_CAP : init_cap);

  (void)__memset(uninit_vec, 0x00, sizeof(vector));

  uninit_vec->_allocator = allocator;

  if (unlikely(!__vector_alloc(uninit_vec, capacity))) {
    return (false);
  }

  __vector_set_elem_size_internal(uninit_vec, elem_size);
  __vector_set_capacity_internal(uninit_vec, capacity);
  __vector_set_destructor_internal(uninit_vec, destroy);

  return (true);
}

/* Returns the number of elements currently held by the vector.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) size_t
vector_length(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (this->_len);
}

/* Returns the size in bytes of one element of the vector.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) size_t
vector_elem_size(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (this->_elem_size);
}

/* Returns the number of bytes currently allocated for the internal
 * buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) size_t
vector_capacity(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (this->_capacity);
}

/* Applies the configured destructor to the element at `position`.
 *
 * Preconditions: `this` is a valid initialised vector, `position` is
 * a valid index in [0, length), and a destructor has been configured.
 */
inline __attribute__((always_inline)) void
vector_index_apply_destructor_unchecked(const vector *this, size_t position) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(this->_destructor != NULL);
  __bug_if_fail__(position < vector_length(this));

  this->_destructor(vector_index_to_ptr_unchecked(this, position));
}

/* Applies the configured destructor to each element in the half-open
 * range `[start, end)`.
 *
 * Preconditions: `this` is a valid initialised vector, `start` and
 * `end` define a valid sub-range of the vector, and a destructor has
 * been configured.
 */
inline __attribute__((always_inline)) void
vector_apply_destructor_in_range_unchecked(const vector *this, size_t start,
                                           size_t end) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(start <= end);
  __bug_if_fail__(end <= vector_length(this));

  size_t remaining;

  remaining = end - start;
  while (remaining--) {
    vector_index_apply_destructor_unchecked(this, start + remaining);
  }
}

/* Returns true when the vector contains no elements.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) bool
vector_is_empty(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (vector_length(this) == 0);
}

/* Returns true when a destructor has been configured for the vector's
 * elements.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) bool
vector_has_destructor(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (this->_destructor != NULL);
}

/* Returns true when the buffer cannot accept another element without
 * being reallocated.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) bool
vector_is_full(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (vector_size_of(this) == vector_capacity(this));
}

/* Returns the number of bytes occupied by the live portion of the
 * buffer. This does not include the unused tail of the allocation.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) size_t
vector_size_of(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (vector_length(this) * vector_elem_size(this));
}

/* These primitive pointer-returning accessors are the lowest layer of
 * the container API: every higher-level operation in this module ends
 * up calling one of them, and they are designed to compile away to a
 * single addition on top of a struct field load. They return NULL only
 * as a signal that the requested element does not exist; the
 * `_unchecked` siblings return the address unconditionally. Both
 * shapes are kept because the vector is used in inner loops where the
 * extra parameter slot of an output-pointer convention would dominate
 * the cost of the access.
 */

/* Returns a pointer to the element at `position`, or NULL if the
 * position is out of bounds. The returned pointer is invalidated by
 * any subsequent operation that may grow the buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) void *
vector_index_to_ptr(const vector *this, size_t position) {
  __bug_if_fail__(this != NULL);

  if (likely(position < vector_length(this))) {
    return (vector_index_to_ptr_unchecked(this, position));
  }

  return (NULL);
}

/* Returns a pointer to the element at `position` without any bound
 * check. The returned pointer is invalidated by any subsequent
 * operation that may grow the buffer.
 *
 * Preconditions: `this` is a valid initialised vector, `position` is
 * a valid index within the buffer's capacity.
 */
__pure1 inline __attribute__((always_inline)) void *
vector_index_to_ptr_unchecked(const vector *this, size_t position) {
  __bug_if_fail__(this != NULL);

  return (((char *)vector_first_to_ptr_unchecked(this) +
           vector_elem_size(this) * position));
}

/* Returns a pointer to the first element of the vector, or NULL if
 * the vector is empty. The returned pointer is invalidated by any
 * subsequent operation that may grow the buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) void *
vector_first_to_ptr(const vector *this) {
  __bug_if_fail__(this != NULL);

  if (likely(!vector_is_empty(this))) {
    return (vector_first_to_ptr_unchecked(this));
  }

  return (NULL);
}

/* Returns a pointer to the first byte of the internal buffer. The
 * returned pointer is invalidated by any subsequent operation that
 * may grow the buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) void *
vector_first_to_ptr_unchecked(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (this->_ptr);
}

/* Returns a pointer to the last element of the vector, or NULL if
 * the vector is empty. The returned pointer is invalidated by any
 * subsequent operation that may grow the buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) void *
vector_last_to_ptr(const vector *this) {
  __bug_if_fail__(this != NULL);

  if (likely(!vector_is_empty(this))) {
    return (vector_last_to_ptr_unchecked(this));
  }

  return (NULL);
}

/* Returns a pointer to the last element of the vector without any
 * bound check. The returned pointer is invalidated by any subsequent
 * operation that may grow the buffer.
 *
 * Preconditions: `this` is a valid initialised non-empty vector.
 */
__pure1 inline __attribute__((always_inline)) void *
vector_last_to_ptr_unchecked(const vector *this) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(!vector_is_empty(this));

  return (vector_index_to_ptr_unchecked(this, vector_length(this) - 1));
}

/* Removes every element from the vector, applying the configured
 * destructor to each one if a destructor was registered. The
 * capacity is preserved.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline void vector_clear(vector *this) {
  __bug_if_fail__(this != NULL);

  if (vector_has_destructor(this)) {
    vector_apply_destructor_in_range_unchecked(this, 0, vector_length(this));
  }

  __vector_set_length_internal(this, 0);
}

/* Clears the vector and releases the internal buffer. After this call
 * the vector is uninitialised and may only be reused by passing it to
 * `vector_init` again.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline void vector_deinit(vector *this) {
  __bug_if_fail__(this != NULL);

  vector_clear(this);
  this->_allocator.release(vector_first_to_ptr_unchecked(this));
}

/* Zeroes the entire internal buffer and releases it. After this call
 * the vector is uninitialised and may only be reused by passing it to
 * `vector_init` again. Intended for buffers that held sensitive data.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline void vector_deinit_zeroized(vector *this) {
  __bug_if_fail__(this != NULL);

  if (likely(vector_capacity(this))) {
    (void)__memset(vector_first_to_ptr_unchecked(this), 0,
                   vector_capacity(this));
  }

  this->_allocator.release(vector_first_to_ptr_unchecked(this));
}

/* Ensures the buffer has room for at least `n` additional elements,
 * growing it if necessary. Growth at least doubles the capacity.
 *
 * Preconditions: `this` is a valid initialised vector.
 * Returns: true on success. On failure the vector is unchanged.
 */
__no_discard bool vector_adjust_cap_if_full(vector *this, size_t n) {
  __bug_if_fail__(this != NULL);

  size_t need;
  size_t twice_capacity;

  need = (n + vector_length(this)) * vector_elem_size(this);

  if (likely(need < vector_capacity(this))) {
    return (true);
  }

  twice_capacity = vector_capacity(this) * 2;
  if (twice_capacity < MIN_CAP) {
    twice_capacity = MIN_CAP;
  }

  return (likely(
      __vector_realloc(this, need > twice_capacity ? need : twice_capacity) !=
      false));
}

/* Ensures the buffer has room for exactly `n` additional elements,
 * growing it only if necessary. Unlike `vector_adjust_cap_if_full` no
 * geometric overshoot is performed.
 *
 * Preconditions: `this` is a valid initialised vector.
 * Returns: true on success. On failure the vector is unchanged.
 */
__no_discard inline bool vector_adjust_exact_cap_if_full(vector *this,
                                                         size_t n) {
  __bug_if_fail__(this != NULL);

  size_t need;

  need = (n + vector_length(this)) * vector_elem_size(this);

  if (likely(need < vector_capacity(this))) {
    return (true);
  }

  return (likely(__vector_realloc(this, need)));
}

/* Appends one element to the end of the vector, copying `elem_size`
 * bytes from `element` into the buffer. Grows the buffer if needed.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points to at least `elem_size` readable bytes that do not overlap
 * the vector's buffer.
 * Returns: true on success. On failure the vector is unchanged.
 */
__no_discard inline bool vector_push(vector *this, const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  if (unlikely(!vector_adjust_cap_if_full(this, 1))) {
    return (false);
  }

  vector_push_within_inner_unchecked(this, element);

  return (true);
}

/* Appends one element to the end of the vector, copying `elem_size`
 * bytes from `element` into the buffer. Grows the buffer if needed.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points to at least `elem_size` readable bytes that do not overlap
 * the vector's buffer.
 *
 * # WARNING
 * this cannot fail and will abort in case of OOM or overflow
 */
inline void vector_push_infaillible(vector *this, const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  if (unlikely(!vector_adjust_cap_if_full(this, 1))) {
    abort();
  }

  vector_push_within_inner_unchecked(this, element);
}

/* Appends one element to the end of the vector without growing the
 * buffer. Returns false if the buffer is already full, in which case
 * any existing pointers into the buffer remain valid.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points to at least `elem_size` readable bytes that do not overlap
 * the vector's buffer.
 */
__no_discard inline bool vector_push_within_inner(vector *this,
                                                  const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  if (unlikely(vector_size_of(this) + vector_elem_size(this) * 1 >
               vector_capacity(this))) {
    return (false);
  }

  vector_push_within_inner_unchecked(this, element);

  return (true);
}

/* Appends one element to the end of the vector without any capacity
 * check.
 *
 * Preconditions: `this` is a valid initialised vector, `element`
 * points to at least `elem_size` readable bytes that do not overlap
 * the vector's buffer, and at least one element of capacity remains.
 */
inline void vector_push_within_inner_unchecked(vector *this,
                                               const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  (void)__memmove(vector_uninitialized_data(this), element,
                  vector_elem_size(this));

  vector_append_from_capacity(this, 1);
}

/* Removes the last element of the vector, applying the configured
 * destructor if one was registered. Popping from an empty vector is a
 * no-op.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline void vector_pop(vector *this) {
  __bug_if_fail__(this != NULL);

  if (unlikely(vector_length(this) == 0)) {
    return;
  }

  if (vector_has_destructor(this)) {
    vector_index_apply_destructor_unchecked(this, vector_length(this) - 1);
  }

  __vector_set_length_internal(this, vector_length(this) - 1);
}

/* Inserts an element before the element at `position`, shifting every
 * subsequent element one slot to the right. Insertion at the current
 * end is not supported by this function; the vector must contain at
 * least one element and `position` must be strictly less than the
 * current length.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points to at least `elem_size` readable bytes that do not overlap
 * the vector's buffer.
 * Returns: true on success. On failure the vector is unchanged.
 */
__no_discard inline bool vector_insert(vector *this, size_t position,
                                       const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  if (unlikely(position >= vector_length(this)) ||
      unlikely(!vector_adjust_cap_if_full(this, 1))) {
    return (false);
  }

  vector_insert_within_inner_unchecked(this, position, element);

  return (true);
}

/* Inserts an element before the element at `position` without growing
 * the buffer. Returns false if `position` is invalid or the buffer is
 * already full; in either case any existing pointers into the buffer
 * remain valid.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points to at least `elem_size` readable bytes that do not overlap
 * the vector's buffer.
 */
__no_discard inline bool
vector_insert_within_inner(vector *this, size_t position, const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  if (unlikely(position >= vector_length(this)) ||
      unlikely((vector_size_of(this) + vector_elem_size(this) * 1) >
               vector_capacity(this))) {
    return (false);
  }

  vector_insert_within_inner_unchecked(this, position, element);

  return (true);
}

/* Inserts an element before the element at `position` without any
 * capacity or bound check.
 *
 * Preconditions: `this` is a valid initialised vector, `position` is
 * a valid index within the current length, `element` points to at
 * least `elem_size` readable bytes that do not overlap the vector's
 * buffer, and at least one element of capacity remains.
 */
inline void vector_insert_within_inner_unchecked(vector *this, size_t position,
                                                 const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  (void)__memmove((char *)vector_index_to_ptr_unchecked(this, position + 1),
                  (char *)vector_index_to_ptr_unchecked(this, position),
                  vector_size_of(this) - position * vector_elem_size(this));

  (void)__memcpy((char *)vector_index_to_ptr_unchecked(this, position), element,
                 vector_elem_size(this));

  vector_append_from_capacity(this, 1);
}

/* Inserts `len` contiguous elements pointed to by `src` into the
 * vector starting at `position`. `position` may equal the current
 * length, in which case the elements are appended. Grows the buffer
 * if needed.
 *
 * Preconditions: `this` is a valid initialised vector and `src`
 * points to at least `len * elem_size` readable bytes that do not
 * overlap the vector's buffer.
 * Returns: true on success. On failure the vector is unchanged.
 */
__no_discard inline bool vector_copy_contiguous(vector *this, size_t position,
                                                const void *src, size_t len) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(src != NULL || len == 0);

  if (unlikely(position > vector_length(this)) ||
      unlikely(!vector_adjust_cap_if_full(this, len))) {
    return (false);
  }

  vector_copy_contiguous_within_inner_unchecked(this, position, src, len);

  return (true);
}

/* Inserts `len` contiguous elements at `position` without growing the
 * buffer. Returns false if `position` is invalid or the buffer is too
 * small; in either case any existing pointers into the buffer remain
 * valid.
 *
 * Preconditions: `this` is a valid initialised vector and `src`
 * points to at least `len * elem_size` readable bytes that do not
 * overlap the vector's buffer.
 */
__no_discard inline bool vector_copy_contiguous_within_inner(vector *this,
                                                             size_t position,
                                                             const void *src,
                                                             size_t len) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(src != NULL || len == 0);

  if (unlikely(position > vector_length(this)) ||
      unlikely((vector_size_of(this) + vector_elem_size(this) * len) >
               vector_capacity(this))) {
    return (false);
  }

  vector_copy_contiguous_within_inner_unchecked(this, position, src, len);

  return (true);
}

/* Inserts `len` contiguous elements at `position` without any
 * capacity or bound check.
 *
 * Preconditions: `this` is a valid initialised vector, `position`
 * is in [0, length], `src` points to at least `len * elem_size`
 * readable bytes that do not overlap the vector's buffer, and the
 * buffer has at least `len` elements of free capacity.
 */
inline void vector_copy_contiguous_within_inner_unchecked(vector *this,
                                                          size_t position,
                                                          const void *src,
                                                          size_t len) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(src != NULL || len == 0);

  if (position < vector_length(this)) {
    (void)__memmove((char *)vector_index_to_ptr_unchecked(this, position + len),
                    (char *)vector_index_to_ptr_unchecked(this, position),
                    vector_elem_size(this) * (vector_length(this) - position));
  }

  (void)__memmove((char *)vector_index_to_ptr_unchecked(this, position), src,
                  vector_elem_size(this) * len);

  vector_append_from_capacity(this, len);
}

/* Inserts an element before the first element of the vector.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points to at least `elem_size` readable bytes that do not overlap
 * the vector's buffer.
 * Returns: true on success. On failure the vector is unchanged.
 */
__no_discard inline __attribute__((always_inline)) bool
vector_pushf(vector *this, const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  if (unlikely(vector_is_empty(this))) {
    return (vector_push(this, element));
  }

  return (vector_insert(this, 0, element));
}

/* Inserts an element before the first element of the vector without
 * growing the buffer.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points to at least `elem_size` readable bytes that do not overlap
 * the vector's buffer.
 * Returns: true on success. On failure the vector is unchanged.
 */
__no_discard inline __attribute__((always_inline)) bool
vector_pushf_within_inner(vector *this, const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  if (unlikely(vector_is_empty(this))) {
    return (vector_push_within_inner(this, element));
  }

  return (vector_insert_within_inner(this, 0, element));
}

/* Inserts an element before the first element of the vector without
 * any capacity or bound check.
 *
 * Preconditions: `this` is a valid initialised vector containing at
 * least one element, `element` points to at least `elem_size`
 * readable bytes that do not overlap the vector's buffer, and at
 * least one element of capacity remains.
 */
inline __attribute__((always_inline)) void
vector_pushf_within_inner_unchecked(vector *this, const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  if (unlikely(vector_is_empty(this))) {
    vector_push_within_inner_unchecked(this, element);
  } else {
    vector_insert_within_inner_unchecked(this, 0, element);
  }
}

/* Removes the first element of the vector. Popping from an empty
 * vector is a no-op.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline __attribute__((always_inline)) void vector_popf(vector *this) {
  __bug_if_fail__(this != NULL);

  vector_remove(this, 0);
}

/* Removes the element at `position`, applying the destructor if one
 * was registered. An out-of-range `position` is a no-op.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline void vector_remove(vector *this, size_t position) {
  __bug_if_fail__(this != NULL);

  if (unlikely(position >= vector_length(this))) {
    return;
  }

  if (vector_has_destructor(this)) {
    vector_index_apply_destructor_unchecked(this, position);
  }

  vector_leak_unchecked(this, position);
}

/* Removes `len` elements starting at `start`, applying the destructor
 * to each one if a destructor was registered. An invalid range is a
 * no-op.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline void vector_remove_range(vector *this, size_t start, size_t len) {
  __bug_if_fail__(this != NULL);

  if (start > vector_length(this) || len > vector_length(this) - start) {
    return;
  }

  if (vector_has_destructor(this)) {
    vector_apply_destructor_in_range_unchecked(this, start, start + len);
  }

  vector_leak_range_unchecked(this, start, len);
}

/* Removes the element at `position` without applying the destructor,
 * transferring ownership of the element to the caller. An
 * out-of-range `position` is a no-op.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline void vector_leak(vector *this, size_t position) {
  __bug_if_fail__(this != NULL);

  if (unlikely(position >= vector_length(this))) {
    return;
  }

  vector_leak_unchecked(this, position);
}

/* Removes the element at `position` without applying the destructor
 * and without any bound check, transferring ownership of the element
 * to the caller.
 *
 * Preconditions: `this` is a valid initialised vector and `position`
 * is a valid index in [0, length).
 */
inline void vector_leak_unchecked(vector *this, size_t position) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(position < vector_length(this));

  size_t tail_bytes;

  tail_bytes = (vector_length(this) - position) * vector_elem_size(this);

  if (likely(position <= vector_length(this))) {
    (void)__memmove(vector_index_to_ptr_unchecked(this, position),
                    vector_index_to_ptr_unchecked(this, position + 1),
                    tail_bytes - vector_elem_size(this));
  }

  __vector_set_length_internal(this, vector_length(this) - 1);
}

/* Removes `len` elements starting at `start` without applying the
 * destructor, transferring ownership of those elements to the caller.
 * An invalid range is a no-op.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
inline void vector_leak_range(vector *this, size_t start, size_t len) {
  __bug_if_fail__(this != NULL);

  if (start > vector_length(this) || len > vector_length(this) - start) {
    return;
  }

  vector_leak_range_unchecked(this, start, len);
}

/* Removes `len` elements starting at `start` without applying the
 * destructor and without any bound check.
 *
 * Preconditions: `this` is a valid initialised vector and `[start,
 * start + len)` is a valid sub-range.
 */
inline void vector_leak_range_unchecked(vector *this, size_t start,
                                        size_t len) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(start + len <= vector_length(this));

  (void)__memmove(vector_index_to_ptr_unchecked(this, start),
                  vector_index_to_ptr_unchecked(this, start + len),
                  (vector_length(this) - (start + len)) *
                      vector_elem_size(this));

  __vector_set_length_internal(this, vector_length(this) - len);
}

/* Swaps the elements at indices `a` and `b` in place, without
 * allocating a temporary.
 *
 * Preconditions: `this` is a valid initialised vector and both `a`
 * and `b` are valid indices in [0, length).
 */
inline void vector_swap_elems(vector *this, size_t a, size_t b) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(a < vector_length(this));
  __bug_if_fail__(b < vector_length(this));

  size_t remaining;
  char *left;
  char *right;

  if (a == b) {
    return;
  }

  remaining = vector_elem_size(this);
  left = vector_index_to_ptr_unchecked(this, a);
  right = vector_index_to_ptr_unchecked(this, b);

  for (; remaining--; ++left, ++right) {
    *left ^= *right;
    *right ^= *left;
    *left ^= *right;
  }
}

/* Reallocates the buffer to a tighter size if more than half of the
 * current capacity is unused.
 *
 * Preconditions: `this` is a valid initialised vector.
 * Returns: true on success or when no shrink was necessary. On
 * allocation failure the vector is unchanged and false is returned.
 */
__no_discard inline bool vector_shrink_to_fit(vector *this) {
  __bug_if_fail__(this != NULL);

  size_t size;

  if (vector_capacity(this) == 0) {
    return (true);
  }

  size = vector_size_of(this);
  if (size >= vector_capacity(this) / 2) {
    return (true);
  }

  return (__vector_realloc(this, size));
}

/* Constructs a vector directly from caller-owned components without
 * computing or validating anything. Intended for transferring an
 * existing buffer into the vector model. The caller is responsible
 * for the consistency of every argument.
 *
 * Preconditions: `uninit_vec` points to writable storage; `ptr` was
 * obtained from `allocator`. `capacity` is the allocation size in
 * bytes; `len * elem_size <= capacity`.
 */
inline void vector_from_raw_parts(vector *uninit_vec,
                                  vector_allocator_t allocator, void *ptr,
                                  size_t elem_size, size_t len, size_t capacity,
                                  void (*destructor)(void *)) {
  __bug_if_fail__(uninit_vec != NULL);
  __bug_if_fail__(allocator.alloc != NULL);
  __bug_if_fail__(allocator.release != NULL);
  __bug_if_fail__(elem_size != 0);
  __bug_if_fail__(len * elem_size <= capacity);

  __vector_set_capacity_internal(uninit_vec, capacity);
  __vector_set_length_internal(uninit_vec, len);
  __vector_set_ptr_internal(uninit_vec, ptr);
  __vector_set_elem_size_internal(uninit_vec, elem_size);
  uninit_vec->_allocator = allocator;
  uninit_vec->_destructor = destructor;
}

/* Constructs a static vector directly from caller owned components without
 * computing or validating anything. Intended for transferring an
 * existing static buffer into the vector model. The caller is responsible
 * for the consistency of every argument.
 *
 * Preconditions: `uninit_vec` points to writable storage. `ptr` is a buffer
 * containing at least `capacity` bytes. `len * elem_size <= capacity`.
 *
 * # IMPORTANT:
 * this vector constructor is intended to be used with static buffers that
 * cannot grow, thus, every pointer within it are stable across the lifetime of
 * the vector. It does not have an allocator, any method that attempts to grow
 * it will fail. It is advised to only use `*within_inner` methods.
 */
inline void vector_from_raw_static_parts(vector *uninit_vec, void *ptr,
                                         size_t elem_size, size_t len,
                                         size_t capacity,
                                         void (*destructor)(void *)) {
  __bug_if_fail__(uninit_vec != NULL);
  __bug_if_fail__(elem_size != 0);
  __bug_if_fail__(len * elem_size <= capacity);

  vector_allocator_t allocator = {
      .alloc = __static_noop_malloc,
      .release = __static_noop_free,
  };

  __vector_set_capacity_internal(uninit_vec, capacity);
  __vector_set_length_internal(uninit_vec, len);
  __vector_set_ptr_internal(uninit_vec, ptr);
  __vector_set_elem_size_internal(uninit_vec, elem_size);
  uninit_vec->_allocator = allocator;
  uninit_vec->_destructor = destructor;
}

/* Returns the offset of the element whose address is `element`,
 * computed from pointer arithmetic against the buffer.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points into the live portion of its buffer. There is no way to
 * verify this from inside the function; mis-use produces meaningless
 * results.
 */
__pure1 inline __attribute__((always_inline)) size_t
vector_elem_get_offset(const vector *this, const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  return (((const char *)element -
           (const char *)vector_first_to_ptr_unchecked(this)));
}

/* Returns the index of the element whose address is `element`,
 * computed from pointer arithmetic against the buffer.
 *
 * Preconditions: `this` is a valid initialised vector and `element`
 * points into the live portion of its buffer. There is no way to
 * verify this from inside the function; mis-use produces meaningless
 * results.
 */
__pure1 inline __attribute__((always_inline)) size_t
vector_elem_get_index(const vector *this, const void *element) {
  __bug_if_fail__(this != NULL);
  __bug_if_fail__(element != NULL);

  return (((const char *)element -
           (const char *)vector_first_to_ptr_unchecked(this)) /
          vector_elem_size(this));
}

/* Returns a pointer to the first byte past the live portion of the
 * buffer (i.e. the slot at `length`). Intended for callers that
 * intend to write a new element by hand and then call
 * `vector_append_from_capacity`. The returned pointer is invalidated
 * by any subsequent operation that may grow the buffer.
 *
 * Preconditions: `this` is a valid initialised vector with at least
 * one element of free capacity beyond `length`.
 */
__pure1 inline __attribute__((always_inline)) void *
vector_uninitialized_data(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (vector_index_to_ptr_unchecked(this, vector_length(this)));
}

/* Returns the number of elements that can be appended before the
 * buffer must be grown.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline size_t vector_uninitialized_length(const vector *this) {
  __bug_if_fail__(this != NULL);

  size_t free_bytes;

  free_bytes = vector_capacity(this) - vector_size_of(this);

  if (free_bytes) {
    free_bytes /= vector_elem_size(this);
  }

  return (free_bytes);
}

/* Returns the number of free bytes beyond the live portion of the
 * buffer.
 *
 * Preconditions: `this` is a valid initialised vector.
 */
__pure1 inline __attribute__((always_inline)) size_t
vector_uninitialized_size_of(const vector *this) {
  __bug_if_fail__(this != NULL);

  return (vector_capacity(this) - vector_size_of(this));
}

/* Increments the visible length by `n`, claiming `n` previously
 * unused slots as live elements. Intended for callers that have
 * written those slots by hand through `vector_uninitialized_data`.
 *
 * Preconditions: `this` is a valid initialised vector, `n` slots
 * beyond the current length have been initialised by the caller,
 * and `length + n <= capacity / elem_size`.
 */
inline __attribute__((always_inline)) void
vector_append_from_capacity(vector *this, size_t n) {
  __bug_if_fail__(this != NULL);

  this->_len += n;
}