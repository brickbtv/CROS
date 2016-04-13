#include "memdetails.h"
#include "assert_shared.h"
#include "string_shared.h"
#include "extern/tlsf/tlsf.h"

#include "sdk/os/process.h"
#include "sdk/os/debug.h"

#include "kernel/kernel/kernel.h"
#include "kernel/process/process.h"

static void * kernel_heap;

void _mem_init(void* start, size_t size, int krn)
{
	if (krn == 1){
		kernel_heap = start;
	}
	//sdk_debug_logf("hk: %x", kernel_heap);
	
	int allocated = init_memory_pool(size, start);

	krn_debugLogf("init mem pool: %d == %d", size, allocated);
}

void show_mem_info(void* pool, int krn){
	if (krn == 1)
		krn_debugLogf("Stat_k: %d %d", get_used_size(pool), get_max_size(pool));
	else {
		sdk_debug_logf("Stat_a: %d %d", get_used_size(pool), get_max_size(pool));
	}
}

int is_kernel(){
	int mode = 0;
	if (krn_is_init() == false || krn_getIdleProcess()->sync_lock == TRUE){
		mode = 1;
	}
		
	return mode;
}

void* malloc(size_t size){
	return _malloc_impl(size, is_kernel());
}

void* _malloc_impl( size_t size, int krn)
{
	if (krn == 1){	// kernel
		uint8_t* ptr = malloc_ex(size, kernel_heap);
		show_mem_info(kernel_heap, krn);
		return ptr;
	} else {		// processes
		void* heapStart = sdk_prc_getHeapPointer();		
		uint8_t* ptr = malloc_ex(size, heapStart);
		show_mem_info(heapStart, krn);
		return ptr;
	}
}

void* realloc(void* oldptr, size_t size){
	return _realloc_impl(oldptr, size, is_kernel());
}

void* _realloc_impl( void* oldptr, size_t size, int krn)
{
	if (krn == 1){	// kernel
		uint8_t* newptr = realloc_ex(oldptr, size, kernel_heap);
		return newptr;
	} else {		// processes
		void* heapStart = sdk_prc_getHeapPointer();		
		uint8_t* newptr = realloc_ex(oldptr, size, heapStart);
		return newptr;
	}
}

void* calloc(size_t size){
	return _calloc_impl(size, is_kernel());
}

void* _calloc_impl( size_t size, int krn)
{
	void* p = malloc(size);
	
	if (p) memset(p, 0, size);
	return p;
}

void free(void* ptr){
	_free_impl(ptr, is_kernel());
}

void _free_impl(void* ptr, int krn)
{
	if (krn == 1){	// kernel
		free_ex(ptr, kernel_heap);
	} else {		// processes
		void* heapStart = sdk_prc_getHeapPointer();		
		free_ex(ptr, heapStart);
	}
}