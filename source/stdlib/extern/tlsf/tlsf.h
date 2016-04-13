#ifndef INCLUDED_tlsf
#define INCLUDED_tlsf

/*
** Two Level Segregated Fit memory allocator, version 3.0.
** Written by Matthew Conte, and placed in the Public Domain.
**	http://tlsf.baisoku.org
**
** Based on the original documentation by Miguel Masmano:
**	http://rtportal.upv.es/rtmalloc/allocators/tlsf/index.shtml
**
** Please see the accompanying Readme.txt for implementation
** notes and caveats.
**
** This implementation was written to the specification
** of the document, therefore no GPL restrictions apply.
*/

#include <stddef_shared.h>


extern size_t init_memory_pool(size_t, void *);
extern size_t get_used_size(void *);
extern size_t get_max_size(void *);
extern void destroy_memory_pool(void *);
extern size_t add_new_area(void *, size_t, void *);
extern void *malloc_ex(size_t, void *);
extern void free_ex(void *, void *);
extern void *realloc_ex(void *, size_t, void *);
extern void *calloc_ex(size_t, size_t, void *);

extern void *tlsf_malloc(size_t size);
extern void tlsf_free(void *ptr);
extern void *tlsf_realloc(void *ptr, size_t size);
extern void *tlsf_calloc(size_t nelem, size_t elem_size);

#endif
