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
	
	/*int allocated = */tlsf_create_with_pool(start, size);//init_memory_pool(size, start);

//	krn_debugLogf("init mem pool: %d == %d. Heap: %x", size, allocated, kernel_heap);
}

void _mem_destroy(void* start){
	tlsf_destroy(start);
}

void show_mem_info(void* pool, int krn){
	if (krn == 1) {
		//krn_debugLogf("Kernel used: %d", get_used_size(pool));
	} else {
		//sdk_debug_logf("App used: %d", get_used_size(pool));
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
	/*if (is_kernel() == 1)
		krn_debugLogf("kernel");
	else {
		sdk_debug_logf("app");
	}*/	
	return _malloc_impl(size, is_kernel());
}

void* _malloc_impl( size_t size, int krn)
{
	if (krn == 1){	// kernel
		if (tlsf_check(kernel_heap))
			krn_debugLogf("TLSF: check failed");
		
		int8_t* ptr = tlsf_malloc(kernel_heap, size);
		if (tlsf_check_pool(kernel_heap))
			krn_debugLogf("TLSFk: check failed");
		//show_mem_info(kernel_heap, krn);
		return ptr;
	} else {		// processes
		void* heapStart = sdk_prc_getHeapPointer();		
		if (tlsf_check_pool(heapStart))
			krn_debugLogf("TLSFa: check failed");
		uint8_t* ptr = tlsf_malloc(heapStart, size);
		tlsf_check(heapStart);
		if (ptr == NULL)
			sdk_debug_logf("Failed to allocate %d bytes.", size);
		//show_mem_info(heapStart, krn);
		return ptr;
	}
}

void* realloc(void* oldptr, size_t size){
	return _realloc_impl(oldptr, size, is_kernel());
}

void* _realloc_impl( void* oldptr, size_t size, int krn)
{
	if (krn == 1){	// kernel
		uint8_t* newptr = tlsf_realloc(kernel_heap, oldptr, size);
		return newptr;
	} else {		// processes
		void* heapStart = sdk_prc_getHeapPointer();		
		uint8_t* newptr = tlsf_realloc(heapStart, oldptr, size);
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
		if (tlsf_check_pool(kernel_heap))
			krn_debugLogf("TLSFk: check failed");
		tlsf_free(kernel_heap, ptr);
		if (tlsf_check_pool(kernel_heap))
			krn_debugLogf("TLSFk: check failed");
		//show_mem_info(kernel_heap, 1);
	} else {		// processes
		void* heapStart = sdk_prc_getHeapPointer();		
		if (tlsf_check_pool(heapStart))
			krn_debugLogf("TLSFa: check failed");
		tlsf_free(heapStart, ptr);
		if (tlsf_check_pool(heapStart))
			krn_debugLogf("TLSFa: check failed");
		//show_mem_info(kernel_heap, 0);
	}
}