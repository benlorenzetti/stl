#ifndef VECLOR_H
#define VECLOR_H

#include <stdlib.h>
#include <string.h>

#define VECLOR_SUCCESS 0
#define VECLOR_ALLOC_FAILURE 1

/*   Constants A and B define the reservation policy for the automatic memory
 * managenment. When the current reserved space x is insufficient, the array
 * is reallocated to a new memory block of size
 *
 *                           x' = A*x + B
 *
 * Reserved space will also shrink according to: x' = (x - B) / A.
 * 
 *   Valid ranges for A and B are:
 *     {A e Real: A >= 1}  (to ensure growth)
 *     {B e Natural: B >= 1}  (to ensure growth for small x)
*/
#define VECLOR_A 1.3   // at large n, allocation size x ~ VECLOR_A^(n)
#define VECLOR_B 1     // minimum allocation size, must be >= 1.

#define VECLOR(template_type, constructor, destructor) \
{                                                      \
  (sizeof(template_type)),                             \
  ((int (*)(void*, const void*))(constructor)),        \
  ((void (*)(void*))(destructor)),                     \
  0,                                                   \
  0,                                                   \
  NULL                                                 \
}

/* Create the type "veclor_copy_f" for a "pointer to a function with generic
 * dest and src pointer parameters and integer return value".
 * Functions cast to this type can be registered as the copy constructor to
 * be used by any "STL" container. And a similar type for destructors.
*/
typedef int (*veclor_copy_f) (void*, const void*); 
typedef void (*veclor_dest_f) (void*);

typedef struct veclor_s {
  int type_size;
  veclor_copy_f copy_constructor;
  veclor_dest_f destructor;
  int alloc_size; // negative alloc_size indicates an explicit reservation that
  int array_size;        // should not be shrunk by the auto_reserve() function
  void* begin;
} veclor;

void* veclor_auto_reserve (veclor *);
void* veclor_at (const veclor*, int);
int veclor_push_back (veclor*, const void*);

int veclor_push_back (veclor* vec, const void* val) {
  // Ensure there is allocated space available for the new value
  if (!veclor_auto_reserve(vec))
    return VECLOR_ALLOC_FAILURE;
  // Generate pointer to the new location and call the copy constructor
  void* new_elem = vec->begin + vec->array_size * vec->type_size;
  if (vec->copy_constructor)
  {
    int cc_return = (*vec->copy_constructor) (new_elem, val);
    if (cc_return)
      return cc_return; // copy constructor failure code
  }
  else // if no user specified constructor, default is a memory copy
  {
    memmove (new_elem, val, vec->type_size);
  }
  // Increment the array size and return
  vec->array_size++;
  return VECLOR_SUCCESS;
}


void* veclor_at (const veclor* vec, int n) {
  if (n >= vec->array_size)
    return NULL; // out of bounds error
  return vec->begin + n * vec->type_size;
}

void* veclor_auto_reserve (veclor* dest) {
  // By default, assume that no resizing or relocation will be done-----------|
  int new_alloc_size = dest->alloc_size;
  void* new_alloc_ptr = dest->begin;
  // Test if more memory needs to be reserved---------------------------------|
  if (dest->array_size == abs(dest->alloc_size))   // Note: negative alloc_size
  {                                     // indicates it was explicitly reserved
    new_alloc_size = VECLOR_A * abs(dest->alloc_size) + VECLOR_B;
    new_alloc_ptr = realloc(dest->begin, new_alloc_size * dest->type_size);
    if (!new_alloc_ptr) // realloc() failure does not disturb original memory
      return NULL;// so return NULL but dont overwrite dest->begin/alloc_size 
  }
  // Test if memory should be released----------------------------------------|
  // It will never be if alloc_size is negative due to an explicit reservation
  else if (dest->array_size < (dest->alloc_size - VECLOR_B) / VECLOR_A)
  {
    int new_size = (dest->alloc_size - VECLOR_B) / VECLOR_A;
    new_alloc_ptr = realloc(dest->begin, new_alloc_size * dest->type_size);
    if (!new_alloc_ptr)
      return dest->begin; // failing to shrink the array isnt really a failure
  }
  // Otherwise the current reservation size is ok so no changes---------------|
  else
  {}
  // Save changes and return pointer to the current location of the data------|
  dest->alloc_size = new_alloc_size;
  return (dest->begin = new_alloc_ptr);
}

#endif
