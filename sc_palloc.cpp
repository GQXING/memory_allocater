#include "sc_palloc.h"
#include <unistd.h>
#include <string.h>
#include "sc_log.h"


size_t sc_pagesize = 0;

//#define SC_MAX_ALLOC_FROM_POOL  (sc_pagesize - 1)

#define SC_MAX_ALLOC_FROM_POOL (sc_pagesize == 0 ? (sc_pagesize = getpagesize() -1) : (sc_pagesize))

#define SC_POOL_ALIGNMENT       16


static void *sc_palloc_block(sc_pool_t *pool, size_t size);
static void *sc_palloc_large(sc_pool_t *pool, size_t size);

//分配size大小按照SC_POOL_ALIGNMENT对齐的内存
sc_pool_t *
sc_create_pool(size_t size)
{
  sc_pool_t  *p;

  p = (sc_pool_t*)sc_memalign(SC_POOL_ALIGNMENT, size);
  if (p == NULL) {
    return NULL;
  }

  p->d.last = (unsigned char *) p + sizeof(sc_pool_t);
  p->d.end = (unsigned char *) p + size;
  p->d.next = NULL;
  p->d.failed = 0;

  size = size - sizeof(sc_pool_t);
  p->max = (size < SC_MAX_ALLOC_FROM_POOL) ? size : SC_MAX_ALLOC_FROM_POOL;

  p->current = p;
  p->large = NULL;

  return p;
}


void
sc_destroy_pool(sc_pool_t *pool)
{
  sc_pool_t          *p, *n;
  sc_pool_large_t    *l;

  for (l = pool->large; l; l = l->next) {

    if (l->alloc) {
      sc_free(l->alloc);
    }
  }

  for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
    sc_free(p);

    if (n == NULL) {
      break;
    }
  }
}


void
sc_reset_pool(sc_pool_t *pool)
{
  sc_pool_t        *p;
  sc_pool_large_t  *l;

  for (l = pool->large; l; l = l->next) {
    if (l->alloc) {
      sc_free(l->alloc);
    }
  }

  for (p = pool; p; p = p->d.next) {
    p->d.last = (unsigned char *) p + sizeof(sc_pool_t);
    p->d.failed = 0;
  }

  pool->current = pool;
  pool->large = NULL;
}

void *
sc_palloc(sc_pool_t *pool, size_t size)
{
  unsigned char      *m;
  sc_pool_t  *p;

  if (size <= pool->max) {
      SC_LOG(sc_log::DEBUG, "before current");
    p = pool->current;

    do {
      m = sc_align_ptr(p->d.last, SC_POOL_ALIGNMENT);

      if ((size_t) (p->d.end - m) >= size) {
        p->d.last = m + size;

        return m;
      }

      p = p->d.next;

    } while (p);

    return sc_palloc_block(pool, size);
  }



  return sc_palloc_large(pool, size);
}


void *
sc_pnalloc(sc_pool_t *pool, size_t size)
{
  unsigned char      *m;
  sc_pool_t  *p;


  if (size <= pool->max) {

    p = pool->current;

    do {
      m = p->d.last;

      if ((size_t) (p->d.end - m) >= size) {
        p->d.last = m + size;

        return m;
      }

      p = p->d.next;

    } while (p);

    return sc_palloc_block(pool, size);
  }


  return sc_palloc_large(pool, size);
}

//按照初始化的时候的size大小来分配。
static void *
sc_palloc_block(sc_pool_t *pool, size_t size)
{
  unsigned char      *m;
  size_t       psize;
  sc_pool_t  *p, *new_;

  psize = (size_t) (pool->d.end - (unsigned char *) pool);

  m = (unsigned char*) sc_memalign(SC_POOL_ALIGNMENT, psize);
  if (m == NULL) {
    return NULL;
  }

  new_ = (sc_pool_t *) m;

  new_->d.end = m + psize;
  new_->d.next = NULL;
  new_->d.failed = 0;

  m += sizeof(sc_pool_data_t);
  m = sc_align_ptr(m, SC_POOL_ALIGNMENT);
  new_->d.last = m + size;

  for (p = pool->current; p->d.next; p = p->d.next) {
    if (p->d.failed++ > 4) {
      pool->current = p->d.next;
    }
  }

  p->d.next = new_;

  return m;
}


static void *
sc_palloc_large(sc_pool_t *pool,size_t size)
{
  void              *p;
  int              n;
  sc_pool_large_t  *large;

  p = sc_alloc(size);
  if (p == NULL) {
    return NULL;
  }

  n = 0;

  for (large = pool->large; large; large = large->next) {
    if (large->alloc == NULL) {
      large->alloc = p;
      return p;
    }

    if (n++ > 3) {
      break;
    }
  }

  large = (sc_pool_large_t  *)sc_palloc(pool, sizeof(sc_pool_large_t));
  if (large == NULL) {
    sc_free(p);
    return NULL;
  }

  large->alloc = p;
  large->next = pool->large;
  pool->large = large;

  return p;
}


void *
sc_pmemalign(sc_pool_t *pool, size_t size, size_t alignment)
{
  void              *p;
  sc_pool_large_t  *large;

  p = sc_memalign(alignment, size);
  if (p == NULL) {
    return NULL;
  }

  large = (sc_pool_large_t  *)sc_palloc(pool, sizeof(sc_pool_large_t));
  if (large == NULL) {
    sc_free(p);
    return NULL;
  }

  large->alloc = p;
  large->next = pool->large;
  pool->large = large;

  return p;
}


int
sc_pfree(sc_pool_t *pool, void *p)
{
  sc_pool_large_t  *l;

  for (l = pool->large; l; l = l->next) {
    if (p == l->alloc) {

      sc_free(l->alloc);
      l->alloc = NULL;

      return 1;
    }
  }

  return 0;
}

void *
sc_pcalloc(sc_pool_t *pool, size_t size)
{
  void *p;

  p = sc_palloc(pool, size);
  if (p) {
    memset(p, 0,size);
  }

  return p;
}
