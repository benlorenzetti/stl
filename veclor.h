#ifndef VECLOR_H
#define VECLOR_H

#define VECLOR_START_SIZE 1
#define VECLOR_ALLOC_BASE 1.5 

#define VECLOR(template_type, constructor, destructor) \
{                                                      \
  (sizeof(template_type)),                             \
  ((void (*)(void*, const void*))(constructor)),       \
  ((void (*)(void*))(destructor)),                     \
  0,                                                   \
  0,                                                   \
  NULL                                                 \
}

/* Create the type "veclor_copy_f" for a "pointer to a function with generic
 * dest and src pointer parameters and no return value".
 * Functions cast to this type can be registered as the copy constructor to
 * be used by any "STL" container. And a similar type for destructors.
*/
typedef void (*veclor_copy_f) (void*, const void*); 
typedef void (*veclor_dest_f) (void*);

typedef struct veclor_s {
  int type_size;
  veclor_copy_f copy_constructor;
  veclor_dest_f destructor;
  int alloc_size;
  int array_size;
  void* begin;
} veclor;

void* reallocation_policy (veclor* dest) {
  if (dest->begin == NULL)
  {
    dest->begin = malloc(VECLOR_START_SIZE * dest->type_size);
    dest->alloc_size = dest->begin ? VECLOR_START_SIZE : 0;
    return dest->begin;
  }
  else if (dest->alloc_size == dest->array_size)
  {
    dest->begin = realloc(VECLOR_ALLOC_BASE * dest->alloc_size);
    dest->alloc_size *= dest->begin ? VECLOR_ALLOC_BASE : 0;
    return dest->begin;
  }
  else if (((dest->alloc_size - VECLOR_START_SIZE)/VECLOR_ALLOC_BASE) > dest->array_size)
  {
    void* new_begin = realloc(dest->alloc_size / VECLOR_ALLOC_BASE);
    if (!new_begin)
      return dest->begin; // realloc() leaves original block untouched on failure
    dest->alloc_size = dest->alloc_size / VECLOR_ALLOC_BASE;
    return (dest->begin = new_begin);
  }
  else
    return dest->begin;
}

#endif
