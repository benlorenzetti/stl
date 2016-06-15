#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>

#define LOR_VECTOR_EXIT_SUCCESS 0
#define LOR_VECTOR_ALLOCATION_FAILURE 1

#define LOR_VECTOR_DOT_NAMESPACE vector
#ifdef LOR_VECTOR_NAMESPACE
  #undef  LOR_VECTOR_DOT_NAMESPACE
  #define LOR_VECTOR_DOT_NAMESPACE LOR_VECTOR_NAMESPACE
#endif

/*   Constants A and B define the reservation policy for the automatic memory
 * managenment. When the current reserved space x is insufficient, the array
 * is reallocated to a new memory block of size
 *                           x' = A*x + B
 * Reserved space will also shrink according to: x' = (x - B) / A.
 * 
 *   Valid ranges for A and B are:
 *     {A e Real: A >= 1}  (to ensure growth)
 *     {B e Natural: B >= 1}  (to ensure growth for small x)
*/
#define LOR_VECTOR_A 1.3
#define LOR_VECTOR_B 1

#define LOR_VECTOR(template_type, constructor, destructor) \
{                                                          \
  (sizeof(template_type)),                                 \
  ((int (*)(void*, const void*))(constructor)),            \
  ((void (*)(void*))(destructor)),                         \
  0,                                                       \
  NULL,                                                    \
  NULL                                                     \
}

/*   Create the type "lor_vetlor_copy_f" for a "pointer to a function with a
 * generic dest and src pointer parameters and integer return value".
 * Functions cast to this type can be registered as the copy constructor to
 * be used by any "GCL" container. And a similar type for destructors.
*/
typedef int (*lor_vector_copy_f) (void*, const void*); 
typedef void (*lor_vector_dest_f) (void*);

typedef struct lor_vector_s {
  int t_size;
  lor_vector_copy_f copy_constructor;
  lor_vector_dest_f destructor;
  int alloc_len; // negative alloc_len indicates an explicit reservation that
        // should not be shrunk by the auto_reserve() function
  void* begin;
  void* end;
} lor_vector;

void* lor_vector_auto_reserve (lor_vector *);
void* lor_vector_at (const lor_vector*, int);
int lor_vector_push_back (lor_vector*, const void*);
int lor_vector_insert (lor_vector*, int, const void*);
int lor_vector_size (const lor_vector*);

typedef struct lor_vector_namespace_s {
  void* (*const auto_reserve)(lor_vector *);
  void* (*const at)(const lor_vector*, int);
  int (*const push_back)(lor_vector*, const void*);
  int (*const insert)(lor_vector*, int, const void*);
  int (*const size)(const lor_vector*);
} lor_vector_namespace;

extern lor_vector_namespace const LOR_VECTOR_DOT_NAMESPACE;

lor_vector_namespace const LOR_VECTOR_DOT_NAMESPACE = {
  lor_vector_auto_reserve,
  lor_vector_at,
  lor_vector_push_back,
  lor_vector_insert
};

/* Begin the Implentation File */

int lor_vector_size (const lor_vector* vec) {
  return (vec->end - vec->begin) / vec->t_size;
}

int lor_vector_insert (lor_vector* vec, int pos, const void* val) {
  // Ensure there is allocated space available for the new array
  if (!lor_vector_auto_reserve(vec))
    return LOR_VECTOR_ALLOCATION_FAILURE;
  // Shift rightwards all elements starting at pos
  if (vec->copy_constructor)
  {
    void* ptr;
    for (ptr=vec->end; ptr> vec->begin + pos*vec->t_size; ptr -= vec->t_size)
    {
      int cc_return = (*vec->copy_constructor) (ptr - vec->t_size, ptr);
      if (cc_return)
        return cc_return;
    }
  }
  else // (no copy constructor specified)
  {
    void* pos_ptr = vec->begin + pos * vec->t_size;
    int copy_byte_length = vec->t_size * (lor_vector_size(vec) - pos);
    memmove (pos_ptr + vec->t_size, pos_ptr, copy_byte_length);
  }
  // Copy the value to the insert position
  if (vec->copy_constructor)
  {
    int cc_ret = (*vec->copy_constructor)(vec->begin+pos*vec->t_size, val);
    if (cc_ret) return cc_ret;
  }
  else
    memmove (vec->begin + pos * vec->t_size, val, vec->t_size);
  // Return success indicator
  return LOR_VECTOR_EXIT_SUCCESS;
}

int lor_vector_push_back (lor_vector* vec, const void* val) {
printf ("entering push_back()\n");
  // Ensure there is allocated space available for the new value
  if (!lor_vector_auto_reserve(vec))
    return LOR_VECTOR_ALLOCATION_FAILURE;
  // Generate pointer to the new location and call the copy constructor
  if (vec->copy_constructor)
  {
    int cc_return = (*vec->copy_constructor) (vec->end, val);
    if (cc_return)
      return cc_return; // copy constructor failure code
  }
  else // (no copy constructor specified)
    memmove (vec->end, val, vec->t_size);
  // Increment the end pointer and return
  vec->end += vec->t_size;
  return LOR_VECTOR_EXIT_SUCCESS;
}

void* lor_vector_at (const lor_vector* vec, int n) {
  if (n >= (vec->end - vec->begin) / vec->t_size)
    return NULL; // out of bounds error
  return vec->begin + n * vec->t_size;
}

void* lor_vector_auto_reserve (lor_vector* dest) {
printf ("entering auto_reserve()...");
  // By default, assume that no resizing or relocation will be done-----------|
  int new_alloc_len = dest->alloc_len;
  void* new_alloc_ptr = dest->begin;
  int array_len = (dest->end - dest->begin) / dest->t_size;
  // Test if more memory needs to be reserved---------------------------------|
  if (array_len == abs(dest->alloc_len))  // note that negative alloc_size
  {                          // indicates it should not be automatically shrunk
    new_alloc_len = LOR_VECTOR_A * abs(dest->alloc_len) + LOR_VECTOR_B;
printf ("grow to new len = %d...", new_alloc_len);
    new_alloc_ptr = realloc(dest->begin, new_alloc_len * dest->t_size);
    if (!new_alloc_ptr) // realloc() failure does not disturb original memory
      return NULL;// so return NULL but dont overwrite dest->begin/alloc_size 
  }
  // Test if memory should be released----------------------------------------|
  // It will not be if alloc_size is negative due to an explicit reservation
  else if (array_len < (dest->alloc_len - LOR_VECTOR_B) / LOR_VECTOR_A)
  {
    new_alloc_len = (dest->alloc_len - LOR_VECTOR_B) / LOR_VECTOR_A;
printf ("shrink to new len = %d...", new_alloc_len);
    new_alloc_ptr = realloc(dest->begin, new_alloc_len * dest->t_size);
    if (!new_alloc_ptr)
      return dest->begin; // failing to shrink the array isnt really a failure
  }
  // Otherwise the current reservation size is ok so no changes---------------|
  else
  {}
  // Save changes and return pointer to the current location of the data------|
  dest->alloc_len = new_alloc_len;
  dest->begin = new_alloc_ptr;
  dest->end = new_alloc_ptr + array_len * dest->t_size;
printf("success.\n");
  return dest->begin;
}

#endif
