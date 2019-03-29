#ifndef SC_PALLOC_H
#define SC_PALLOC_H

#include <stdlib.h>

#include "sc_alloc.h"
/*---------------------------------------------------------------------
 * this is used to alloc and free together
   int main(){

   sc_pool_t *pool = sc_create_pool(1024);

    int i=0;

    for(i=0;i<10000;i++){

        sc_pcalloc (pool,800 * 10);

    }

    sleep(5);

    sc_destroy_pool (pool);

    sleep(5);
}
------------------------------------------------------------------------*/
typedef struct sc_pool_large_s sc_pool_large_t;

typedef struct sc_pool_s sc_pool_t;

struct sc_pool_large_s {
    sc_pool_large_t *next;
    void *alloc;
};

typedef struct {
    unsigned char *last;
    unsigned char *end;
    sc_pool_t *next;
    unsigned int failed;
} sc_pool_data_t;

struct sc_pool_s {
    sc_pool_data_t d;
    size_t max;
    sc_pool_t *current;
    sc_pool_large_t *large;
};


/*  ======================================
 *  ++++++++  Library Open API  ++++++++++
 *  ======================================
 */


void *sc_alloc (size_t size);

void *sc_calloc (size_t size);

sc_pool_t *sc_create_pool (size_t size);

void sc_destroy_pool (sc_pool_t *pool);

void sc_reset_pool (sc_pool_t *pool);

void *sc_palloc (sc_pool_t *pool, size_t size);

void *sc_pnalloc (sc_pool_t *pool, size_t size);

void *sc_prealloc (sc_pool_t *pool, void *p, size_t size);

void *sc_pcalloc (sc_pool_t *pool, size_t size);

void *sc_pmemalign (sc_pool_t *pool, size_t size, size_t alignment);

int sc_pfree (sc_pool_t *pool, void *p);

#endif // SC_PALLOC_H
