#include "pool_tests.h"

/* BEGIN TEST HELPERS */

void pool_deinit(void) {
	// Zero global pool heap object
	for(int i = 0; i < HEAP_SIZE; i++) {
		g_pool_heap[i] = 0;
	}

	// Zero pool controller attributes
	for(int i = 0; i < MAX_POOLS; i++) {
		pool_controller.pool_begin_indices[i] = 0;
		pool_controller.pool_end_indices[i] = 0;
		pool_controller.ppool_allocators[i] = 0; 
	}
}

bool verify_heap_init_base(const size_t* block_sizes, size_t block_size_count) {
	bool correct = true; 

	// Force correctness to 0 if any of the check fails
	correct &= verify_heap_init_pool_controller(block_sizes, block_size_count);
	correct &= verify_heap_init_block_references(block_sizes, block_size_count);
	return correct; 
}

bool verify_heap_init_pool_controller(const size_t* block_sizes, size_t block_size_count) {
	bool correct = true;
	
	if(block_size_count != pool_controller.num_pools) {
		correct = false;
	}

	size_t pool_begin, pool_end;
	uint8_t* ppool_allocator; 
	for(size_t i = 0; i < block_size_count; i++) {
		pool_begin = pool_controller.pool_begin_indices[i];
		pool_end   = pool_controller.pool_end_indices[i];
		ppool_allocator = pool_controller.ppool_allocators[i];

		// Verify pool's begin bounds
		if(pool_begin != (i * HEAP_SIZE / block_size_count)) {
			correct = false; 
		}

		// Verify pool's end bounds
		if(pool_end != ((i+1) * HEAP_SIZE / block_size_count - block_sizes[i])) {
			correct = false; 
		}

		// Verify that pool's allocator points to beginning of the pool
		if(ppool_allocator != &g_pool_heap[pool_begin]) {
			correct = false; 
		}
	}

	return correct;
}

bool verify_heap_init_block_references(const size_t* block_sizes, size_t block_size_count) {
	bool correct = true;
	
	uint32_t pool_begin, last_block, next_block; 
	for(size_t i = 0; i < block_size_count; i++) {
		pool_begin = pool_controller.pool_begin_indices[i];
		last_block   = pool_controller.pool_end_indices[i];

		// Re-construct 16-bit address based on two 8-bit elements
		next_block = (g_pool_heap[pool_begin+1] << 8) | g_pool_heap[pool_begin];

		// Verify behaviour for pools that can hold more than one block
		if(block_sizes[i] < HEAP_SIZE / block_size_count) {
			// Check reference between first two blocks blocks within pool
			if(next_block != (pool_begin + block_sizes[i])) {
				correct = false; 
			}

			// Check that final block refers back to first block in pool
			next_block = (g_pool_heap[last_block+1] << 8) | g_pool_heap[last_block]; 
			if(next_block != pool_begin) {
				correct = false;
			}
		}
		// Verify behaviour for pools that can only hold one block
		else if (next_block != pool_begin){
			correct = false;
		}
	}
	return correct;
}

/* END TEST HELPERS */


/* BEGIN TEST RUNNER */

void TestRunner(void) {
	// Test input verification
	assert(!verify_inputs_small_none());
	assert(verify_inputs_small_single());
	assert(verify_inputs_small_multiple());

	assert(verify_inputs_large_single());
	assert(verify_inputs_large_multiple());

	assert(verify_inputs_mixed_multiple());
	assert(verify_inputs_max_single());
	assert(!verify_inputs_exceed_single());
	
	// Test pool init
	assert(pool_init_small_single());
	assert(pool_init_small_multiple());

	assert(pool_init_large_single());
	assert(pool_init_large_multiple());

	assert(pool_init_mixed_multiple());
	assert(pool_init_max_single());
}

/* END TEST RUNNER */


/* BEGIN Heap Inputs Verification Tests */

bool verify_inputs_small_none(void) {
	size_t block_sizes[] = {4};
	return verify_heap_inputs(block_sizes, 0);
}

bool verify_inputs_small_single(void) {
	size_t block_sizes[] = {4};
	return verify_heap_inputs(block_sizes, 1);
}

bool verify_inputs_small_multiple(void) {
	size_t block_sizes[] = {2, 4, 8, 16};
	return verify_heap_inputs(block_sizes, 4);
}

bool verify_inputs_large_single(void) {
	size_t block_sizes[] = {65536};
	return verify_heap_inputs(block_sizes, 1);
}

bool verify_inputs_large_multiple(void) {
	size_t block_sizes[] = {16384, 32768};
	return verify_heap_inputs(block_sizes, 2);
}

bool verify_inputs_mixed_multiple(void) {
	size_t block_sizes[] = {2, 128, 1024, 16384};
	return verify_heap_inputs(block_sizes, 4);
}

bool verify_inputs_max_single(void) {
	size_t block_sizes[] = {65536};
	return verify_heap_inputs(block_sizes, 1);
}

bool verify_inputs_exceed_single(void) {
	size_t block_sizes[] = {131072};
	return verify_heap_inputs(block_sizes, 1);
}

/* END Heap Inputs Verification Tests */


/* BEGIN Pool Initialization Tests */

bool pool_init_base(const size_t* block_sizes, size_t block_size_count) {
	bool pass = true; 

	pool_deinit(); // Zero global static heap object

	pass = pool_init(block_sizes, block_size_count);
	pass = verify_heap_init_base(block_sizes, block_size_count);

	return pass;
}

bool pool_init_small_single(void) {
	size_t block_sizes[] = {4};
	return pool_init_base(block_sizes, 1);
}

bool pool_init_small_multiple(void) {
	size_t block_sizes[] = {2, 4, 8, 16};
	return pool_init_base(block_sizes, 4);
}

bool pool_init_large_single(void) {
	size_t block_sizes[] = {32768};
	return pool_init_base(block_sizes, 1);
}

bool pool_init_large_multiple(void) {
	size_t block_sizes[] = {32768, 16384};
	return pool_init_base(block_sizes, 2);
}

bool pool_init_mixed_multiple(void) {
	size_t block_sizes[] = {2, 128, 1024, 16384};
	return pool_init_base(block_sizes, 4);
}

bool pool_init_max_single(void) {
	size_t block_sizes[] = {65536};
	return pool_init_base(block_sizes, 1);
}

/* END Pool Initialization Tests */
