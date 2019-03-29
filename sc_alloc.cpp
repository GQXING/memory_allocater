#include "sc_alloc.h"
#include<string.h>

void *
sc_alloc(size_t size)
{
  void  *p;

  p = malloc(size);

  return p;
}


void *
sc_calloc(size_t size)
{
  void  *p;

  p = sc_alloc(size);

  if (p) {
    memset(p,0,size);
  }

  return p;
}

void *sc_realloc(void *p, size_t size){

  if(p) {
    return realloc (p, size);
  }

  return NULL;

}

void *
sc_memalign(size_t alignment, size_t size)
{
  void  *p;
  int    err;

  err = posix_memalign(&p, alignment, size);

  return p;
}
