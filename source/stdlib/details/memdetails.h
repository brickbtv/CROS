#ifndef _memdetails_shared_h_
#define _memdetails_shared_h_

#include "stddef_shared.h"



// Called by the OS to initialize a process's head
void _mem_init(void* start, size_t size, int krn);
//
// NOTE: Don't use these directly . Use the macros bellow (e.g: malloc)
//
void* _malloc_impl(size_t, int);
void* _realloc_impl(void* ptr, size_t size, int);
void* _calloc_impl(size_t, int);
void  _free_impl(void* ptr, int);

void* malloc(size_t);
void free(void* ptr);
void* realloc(void* ptr, size_t);
void* calloc(size_t);

void show_mem_info(void* pool, int krn);

#endif
