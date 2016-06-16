#ifndef LOR_VECTOR_H
#define LOR_VECTOR_H

/*  Copyright (c) 2016 Ben Lorenzetti
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

/*  This header file is a part of a generic container library for doing
 *  "object-oriented" programming in C. A lor_vector stores metadata about, and
 *  points to an array which is automatically resized as needed by lor_vector
 *  functions. Stuctures of any size may be stored and a copy constructor and
 *  destructor can be registered with the container. The functionality is
 *  like C++'s standard vector class.
 *
 *  The following return codes are defined for functions which return an
 *  integer. If you write your own copy constructor functions, they should
 *  return LOR_VECTOR_EXIT_SUCCESS or an error code of your own choosing
 *  outside the range of the library error codes defined here. 
 */

#define LOR_VECTOR_EXIT_SUCCESS 0
#define LOR_VECTOR_ALLOCATION_FAILURE 1

/*  To avoid namespace conflicts with outside code, all functions, structures,
 *  and typedefs declared in this library are prefixed by "lor_vector_" and 
 *  all macros by "LOR_VECTOR_". Functions can be called using their full
 *  prefix name or through a namespace struct provided for shortening code.
 *  The default namespace is "lor_vector" but this can be changed by defining
 *  LOR_VECTOR_NAMESPACE in your code.
 *  
 *  For example, the push back function could be called the following ways:
 *
 *    1.   lor_vector_push_back();
 *
 *    2.   lor_vector.push_back();
 *
 *    3.   #define LOR_VECTOR_NAMESPACE vec
 *         ...
 *         vec.push_back();
 */

#define LOR_VECTOR_DOT_NAMESPACE lor_vector
#ifdef LOR_VECTOR_NAMESPACE
  #undef  LOR_VECTOR_DOT_NAMESPACE
  #define LOR_VECTOR_DOT_NAMESPACE LOR_VECTOR_NAMESPACE
#endif

struct lor_vector_namespace_s;
extern struct lor_vector_namespace_s const LOR_VECTOR_DOT_NAMESPACE;

/*  Constants A and B define the reservation policy for the automatic memory
 *  managenment. When the current reserved space x is insufficient, the array
 *  is reallocated to a new memory block of size
 *
 *                           x' = A*x + B
 *
 *  Reserved space will also shrink according to: x' = (x - B) / A.
 * 
 *   Valid ranges for A and B are:
 *     {A e Real: A >= 1}  (to ensure growth)
 *     {B e Natural: B >= 1}  (to ensure growth for small x)
*/

#define LOR_VECTOR_A 1.3
#define LOR_VECTOR_B 1

/*  The type "lor_vetlor_copy_f" is defined for a "pointer to a function with a
 *  generic dest and src pointer parameters and integer return value".
 *  Functions cast to this type can be registered as the copy constructor to be
 *  used by any container. And a similar type for destructors.
*/

typedef int (*lor_vector_copy_f) (void*, const void*); 
typedef void (*lor_vector_dest_f) (void*);


#define LOR_VECTOR(template_type, constructor, destructor) \
{                                                          \
  (sizeof(template_type)),                                 \
  ((int (*)(void*, const void*))(constructor)),            \
  ((void (*)(void*))(destructor)),                         \
  0,                                                       \
  NULL,                                                    \
  NULL                                                     \
}

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

struct lor_vector_namespace_s {
  void* (*const auto_reserve)(lor_vector *);
  void* (*const at)(const lor_vector*, int);
  int (*const push_back)(lor_vector*, const void*);
  int (*const insert)(lor_vector*, int, const void*);
  int (*const size)(const lor_vector*);
};

struct lor_vector_namespace_s const LOR_VECTOR_DOT_NAMESPACE ={
  lor_vector_auto_reserve,
  lor_vector_at,
  lor_vector_push_back,
  lor_vector_insert
};

/* Begin the Implentation File */

#include <stdlib.h>
#include <string.h>

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
