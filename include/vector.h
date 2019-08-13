#ifndef _VEC_H_
#define _VEC_H_
// Based on https://github.com/graphitemaster/cvec

#include <string.h>
#include <stdlib.h>

typedef struct
{
    size_t allocated;
    size_t used;
} vector_t;

#define vector(x) x *

/* Attempts to grow [VECTOR] by [MORE]*/
#define vector_try_grow(VECTOR, MORE) \
    (((!(VECTOR) || vector_meta(VECTOR)->used + (MORE) >= vector_meta(VECTOR)->allocated)) ? (void)vec_grow(((void **)&(VECTOR)), (MORE), sizeof(*(VECTOR))) : (void)0)

/* Get the metadata block for [VECTOR] */
#define vector_meta(VECTOR) \
    ((vector_t *)(((unsigned char *)(VECTOR)) - sizeof(vector_t)))

/* Deletes [VECTOR] and sets it to NULL */
#define vector_free(VECTOR) \
    ((void)((VECTOR) ? (vec_delete((void *)(VECTOR)), (VECTOR) = NULL) : 0))

/* Pushes back [VALUE] into [VECTOR] */
#define vector_push(VECTOR, VALUE) \
    (vector_try_grow((VECTOR), 1), (VECTOR)[vector_meta(VECTOR)->used] = (VALUE), vector_meta(VECTOR)->used++)

/* Get the size of [VECTOR] */
#define vector_size(VECTOR) \
    ((VECTOR) ? vector_meta(VECTOR)->used : 0)

/* Get the capacity of [VECTOR] */
#define vector_capacity(VECTOR) \
    ((VECTOR) ? vector_meta(VECTOR)->allocated : 0)

/* Resize [VECTOR] to accomodate [SIZE] more elements */
#define vector_resize(VECTOR, SIZE)                                          \
    (vector_try_grow((VECTOR), (SIZE)), vector_meta(VECTOR)->used += (SIZE), \
     &(VECTOR)[vector_meta(VECTOR)->used - (SIZE)])

/* Get the last element in [VECTOR] */
#define vector_last(VECTOR) \
    ((VECTOR)[vector_meta(VECTOR)->used - 1])

#define vector_lastn(VECTOR, N) \
    (&(VECTOR)[vector_meta(VECTOR)->used - N])

/* Pop an element off the back of [VECTOR] */
#define vector_pop(VECTOR) \
    ((VECTOR)[--vector_meta(VECTOR)->used])
//((void)(vector_meta(VECTOR)->used -= 1))

/* Shrink the size of [VECTOR] down to [SIZE] */
#define vector_shrinkto(VECTOR, SIZE) \
    ((void)(vector_meta(VECTOR)->used = (SIZE)))

/* Shrink [VECTOR] down by [AMOUNT] */
#define vector_shrinkby(VECTOR, AMOUNT) \
    ((void)(vector_meta(VECTOR)->used -= (AMOUNT)))

/* Append to [VECTOR], [COUNT] elements from [POINTER] */
#define vector_append(VECTOR, COUNT, POINTER) \
    ((void)(memcpy(vector_resize((VECTOR), (COUNT)), (POINTER), (COUNT) * sizeof(*(POINTER)))))

/* Remove from [VECTOR], [COUNT] elements starting from [INDEX] */
#define vector_remove(VECTOR, INDEX, COUNT)                                               \
    ((void)(memmove((VECTOR) + (INDEX), (VECTOR) + (INDEX) + (COUNT),                     \
                    sizeof(*(VECTOR)) * (vector_meta(VECTOR)->used - (INDEX) - (COUNT))), \
            vector_meta(VECTOR)->used -= (COUNT)))

static inline void vec_grow(void **vector, size_t more, size_t type_size)
{
    vector_t *meta = vector_meta(*vector);
    size_t count = 0;
    void *data = NULL;

    if (*vector)
    {
        count = 2 * meta->allocated + more;
        data = realloc(meta, type_size * count + sizeof *meta);
    }
    else
    {
        count = more + 1;
        data = malloc(type_size * count + sizeof *meta);
        ((vector_t *)data)->used = 0;
    }

    meta = (vector_t *)data;
    meta->allocated = count;
    *vector = meta + 1;
}

static inline void vec_delete(void *vector)
{
    free(vector_meta(vector));
}

#endif
