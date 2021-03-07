#include "pool_alloc.h"

static uint8_t g_pool_heap[HEAP_SIZE];
static pool_controller_t pool_controller; 


bool verify_heap_inputs(const size_t* block_sizes, size_t block_size_count) {
	bool valid_inputs = true;

	if(!block_size_count || (block_size_count > MAX_POOLS)) {
		printf("Invalid block_size_count\n");
		valid_inputs = false; 
	}

	// Check for block sizes exceeding pool size
	uint16_t pool_sizes = HEAP_SIZE / block_size_count; 
	for(size_t i = 0; i < block_size_count; i++) {
		if(block_sizes[i] > pool_sizes) {
			printf("Invalid block size (%zu)\n", block_sizes[i]);
			valid_inputs = false; 
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
		pool_end_idx = (i+1) * (HEAP_SIZE / block_size_count) - 1;

		pool_controller.pool_begin_indices[i] = pool_begin_idx;
		pool_controller.pool_end_indices[i] = pool_end_idx;

		// Initialize pool pointers
		pool_controller.ppool_allocators[i] = &g_pool_heap[pool_begin_idx];
	}

	uint16_t block_count = 1; 
	uint32_t pool_begin, pool_end, next_block;
	// Use pool controller to populate heap map
	for(size_t i = 0; i < block_size_count; i++) {
		pool_begin = pool_controller.pool_begin_indices[i];
		pool_end   = pool_controller.pool_end_indices[i]; 

		for(size_t j = pool_begin; j < pool_end; j += block_sizes[i]) {
			
			next_block = pool_begin + block_sizes[i] * block_count; 
			if(next_block < pool_end) {
				// Heap must store 16-bit addresses in 2 8-bit elements
				g_pool_heap[j]   = next_block & 0xFF; // Lower byte
				g_pool_heap[j+1] = next_block >> 8;   // Upper byte
			}
			else {
				/* Last block in a pool points back to its pool_default_idx
				 * to indicate the end of the pool. 
				 *
				 * NOTE: Memory cannot be allocated in this block, since it is
				 * 		 required to indicate a full pool. 
				 */
				g_pool_heap[j] = pool_begin; 
			}
			block_count++; 
		}
		block_count = 1;
	}

	return true; 
}

void* pool_malloc(size_t n)
{
	// Check for invalid sizes
}

void pool_free(void* ptr)
{
 // Implement me!
}

int main() {
	const size_t block_sizes[] = {32, 1024};
	size_t block_size_count = sizeof(block_sizes)/sizeof(block_sizes[0]); 

	assert(pool_init(block_sizes, block_size_count));
	return 0; 
}