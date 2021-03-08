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
	for(size_t i = 0; i < block_size_count; i++) {

		// Distribute the heap evenly between blocks
		pool_begin_idx = i * (HEAP_SIZE / block_size_count); 
		pool_end_idx = (i+1) * (HEAP_SIZE / block_size_count) - block_sizes[i];

		pool_controller.pool_begin_indices[i] = pool_begin_idx;
		pool_controller.pool_end_indices[i] = pool_end_idx;

		// Initialize pool pointers
		pool_controller.ppool_allocators[i] = &g_pool_heap[pool_begin_idx];
	}
	pool_controller.num_pools = block_size_count;

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
	/* Changes made: 
	 *
	 * 1. Added assumption 2 for inputs
	 * 2. Added num_pools and block_sizes to pool controller
	 */
	size_t pool_idx = 0;
	for(; pool_idx < pool_controller.num_pools; pool_idx++) {
		if(pool_controller.)
	}
}

void pool_free(void* ptr)
{
 // Check for invalid ptrs
}

int main() {
	// size_t block_sizes[] = {65536};
	// pool_init(block_sizes, 1);
	// verify_heap_init_correctness(block_sizes, 1);

	TestRunner();
	return 0; 
}