#include "pool_alloc.h"
#include "pool_tests.c"


void print_heap_range(size_t idx, size_t padding) {
	for(size_t i = idx - padding; i < idx + padding; i++) {
			printf("%zu: %d\n", i, g_pool_heap[i]);
		}
}

bool verify_heap_inputs(const size_t* block_sizes, size_t block_size_count) {
	bool valid_inputs = true;

	if(!block_size_count || (block_size_count > MAX_POOLS)) {
		valid_inputs = false; 
	}
	else {
		// Check for block sizes exceeding pool size
		size_t pool_sizes = HEAP_SIZE / block_size_count; 
		for(size_t i = 0; i < block_size_count; i++) {
			if(block_sizes[i] > pool_sizes) {
				valid_inputs = false; 
			}
		}
	}
	return valid_inputs; 
}

bool pool_init(const size_t* block_sizes, size_t block_size_count)
{
	if(!verify_heap_inputs(block_sizes, block_size_count)) {
		return false;
	}

	uint16_t pool_begin_idx; 
	uint16_t pool_end_idx;

	// Initialize pool controller
	pool_controller.num_pools = block_size_count; 
	for(size_t i = 0; i < block_size_count; i++) {
		// Save block sizes to global state for use in pool_alloc() and pool_free()
		pool_controller.block_sizes[i] = block_sizes[i];

		// Distribute the heap evenly between blocks
		pool_begin_idx = i * (HEAP_SIZE / block_size_count); 
		pool_end_idx = (i+1) * (HEAP_SIZE / block_size_count) - block_sizes[i];
		pool_controller.pool_begin_indices[i] = pool_begin_idx;
		pool_controller.pool_end_indices[i] = pool_end_idx;

		// Initialize pool allocators and set pools to empty by default
		pool_controller.pool_full[i] = false; 
		pool_controller.pool_allocators[i] = pool_begin_idx;
	}

	uint16_t block_count = 1; 
	uint32_t pool_begin, pool_end, next_block;
	// Use pool controller to populate heap map
	for(size_t i = 0; i < block_size_count; i++) {
		pool_begin = pool_controller.pool_begin_indices[i];
		pool_end   = pool_controller.pool_end_indices[i]; 

		for(size_t j = pool_begin; j < pool_end + 1; j += block_sizes[i]) {
			next_block = pool_begin + block_sizes[i] * block_count; 
			
			if(next_block <= pool_end) {
				// Heap must store 16-bit addresses in 2 8-bit elements
				g_pool_heap[j]   = next_block & 0xFF; // Lower byte
				g_pool_heap[j+1] = next_block >> 8;   // Upper byte
			}
			else {
				/* Last block in a pool points back to its pool_begin
				 * index to indicate the end of the pool. 
				 */
				g_pool_heap[j] = pool_begin & 0xFF;   // Lower byte 
				g_pool_heap[j+1]   = pool_begin >> 8; // Upper byte
			}
			block_count++; 
		}
		block_count = 1;
	}
	return true; 
}

void* pool_malloc(size_t n)
{
	void* store_addr = NULL; 
	uint16_t store_idx; 

	// Find first pool index that can store object of size n
	size_t pool_idx = 0;
	for(; pool_idx < pool_controller.num_pools; pool_idx++) {
		// Possible storage found
		if(n <= pool_controller.block_sizes[pool_idx]) {
			// Sufficient space in current pool
			if(!pool_controller.pool_full[pool_idx]) {
				store_idx = pool_controller.pool_allocators[pool_idx];
				store_addr = &g_pool_heap[store_idx]; 
				break; 
			}
		}
	}	

	// Update pool_allocators and set pool full if necessary
	if(store_addr) {
		// Reconstruct index for next block based on two 8-bit values
		uint16_t next_block_idx = (g_pool_heap[store_idx+1] << 8) | g_pool_heap[store_idx];

		// Check for pool full indication (i.e. loopback to first block in pool)
		if(next_block_idx == pool_controller.pool_begin_indices[pool_idx]) {
			
			printf("Filled last block in pool %zu\n", pool_idx);
			
			pool_controller.pool_allocators[pool_idx] = next_block_idx;
			pool_controller.pool_full[pool_idx] = true; 
		}
		else {
			pool_controller.pool_allocators[pool_idx] = next_block_idx;
		}

		printf("pool %zu malloc: %p\n", pool_idx, store_addr);
	}
	else {
		printf("Not enough space in pools for block size %zu\n", n); 
	}

	return store_addr; 
}

void pool_free(void* ptr)
{
	if(ptr) {
		// Find which pool the memory belongs to
		uint16_t pool_idx = 0, next_pool_begin_idx;
		for(; pool_idx < pool_controller.num_pools - 1; pool_idx++) {

			// Pool is found when ptr address < next pool's begin address
			next_pool_begin_idx = pool_controller.pool_begin_indices[pool_idx + 1];
			if(ptr < (void*)&g_pool_heap[next_pool_begin_idx]) {
				break;
			}
		}

		// Toggle pool full flag if necessary
		if(pool_controller.pool_full[pool_idx]) {
			pool_controller.pool_full[pool_idx] = false; 
		}

		// The freed block now points to the allocation pointer
		uint16_t pa = pool_controller.pool_allocators[pool_idx];
		*(uint16_t*)ptr = pa;

		// The allocation pointer now points to the freed block
		uint16_t ptr_idx = ((uint8_t*)ptr - &g_pool_heap[0]);
		pool_controller.pool_allocators[pool_idx] = ptr_idx; 
	}
}

int main() {
	TestRunner();
	return 0; 
}