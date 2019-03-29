#ifndef SC_ALLOC_H
#define SC_ALLOC_H
#include <stdlib.h>
void *sc_alloc(size_t size);
void *sc_calloc(size_t size);
void *sc_realloc(void *p, size_t size);

#define sc_free          free

#define sc_align_ptr(p, a)                                                   \
  (unsigned char *) (((unsigned long int ) (p) + ((unsigned long int ) a - 1)) & ~((unsigned long int ) a - 1))

void *sc_memalign(size_t alignment, size_t size);


#endif // SC_ALLOC_H
