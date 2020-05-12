#ifndef _VEC_H_
#define _VEC_H_
// Based on https://github.com/graphitemaster/cvec

#include <string.h>
#include <stdlib.h>

typedef struct
{
    size_t allocated;
    size_t used;
} sl_vector_t;

#define sl_vector(x) x *

/* Attempts to grow [VECTOR] by [MORE]*/
#define sl_vector_try_grow(VECTOR, MORE) \
    (((!(VECTOR) || sl_vector_meta(VECTOR)->used + (MORE) >= sl_vector_meta(VECTOR)->allocated)) ? (void)vec_grow(((void **)&(VECTOR)), (MORE), sizeof(*(VECTOR))) : (void)0)

/* Get the metadata block for [VECTOR] */
#define sl_vector_meta(VECTOR) \
    ((sl_vector_t *)(((unsigned char *)(VECTOR)) - sizeof(sl_vector_t)))

/* Deletes [VECTOR] and sets it to NULL */
#define sl_vector_free(VECTOR) \
    ((void)((VECTOR) ? (vec_delete((void *)(VECTOR)), (VECTOR) = NULL) : 0))

/* Pushes back [VALUE] into [VECTOR] */
#define sl_vector_push(VECTOR, VALUE) \
    (sl_vector_try_grow((VECTOR), 1), (VECTOR)[sl_vector_meta(VECTOR)->used] = (VALUE), sl_vector_meta(VECTOR)->used++)

/* Get the size of [VECTOR] */
#define sl_vector_size(VECTOR) \
    ((VECTOR) ? sl_vector_meta(VECTOR)->used : 0)

/* Get the capacity of [VECTOR] */
#define sl_vector_capacity(VECTOR) \
    ((VECTOR) ? sl_vector_meta(VECTOR)->allocated : 0)

/* Resize [VECTOR] to accomodate [SIZE] more elements */
#define sl_vector_resize(VECTOR, SIZE)                                          \
    (sl_vector_try_grow((VECTOR), (SIZE)), sl_vector_meta(VECTOR)->used += (SIZE), \
     &(VECTOR)[sl_vector_meta(VECTOR)->used - (SIZE)])

/* Get the last element in [VECTOR] */
#define sl_vector_last(VECTOR) \
    ((VECTOR)[sl_vector_meta(VECTOR)->used - 1])

#define sl_vector_lastn(VECTOR, N) \
    (&(VECTOR)[sl_vector_meta(VECTOR)->used - N])

/* Pop an element off the back of [VECTOR] */
#define sl_vector_pop(VECTOR) \
    ((VECTOR)[--sl_vector_meta(VECTOR)->used])
//((void)(sl_vector_meta(VECTOR)->used -= 1))

/* Shrink the size of [VECTOR] down to [SIZE] */
#define sl_vector_shrinkto(VECTOR, SIZE) \
    ((void)(sl_vector_meta(VECTOR)->used = (SIZE)))

/* Shrink [VECTOR] down by [AMOUNT] */
#define sl_vector_shrinkby(VECTOR, AMOUNT) \
    ((void)(sl_vector_meta(VECTOR)->used -= (AMOUNT)))

/* Append to [VECTOR], [COUNT] elements from [POINTER] */
#define sl_vector_append(VECTOR, COUNT, POINTER) \
    ((void)(memcpy(sl_vector_resize((VECTOR), (COUNT)), (POINTER), (COUNT) * sizeof(*(POINTER)))))

/* Remove from [VECTOR], [COUNT] elements starting from [INDEX] */
#define sl_vector_remove(VECTOR, INDEX, COUNT)                                               \
    ((void)(memmove((VECTOR) + (INDEX), (VECTOR) + (INDEX) + (COUNT),                     \
                    sizeof(*(VECTOR)) * (sl_vector_meta(VECTOR)->used - (INDEX) - (COUNT))), \
            sl_vector_meta(VECTOR)->used -= (COUNT)))

#define vec_delete(VECTOR) free(sl_vector_meta(VECTOR))

void vec_grow(void **vector, size_t more, size_t type_size);

#endif
