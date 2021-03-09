#include "pool_tests.h"

/* BEGIN TEST HELPERS */

void pool_deinit(void) {
	// Zero global pool heap object
	for(int i = 0; i < HEAP_SIZE; i++) {
		g_pool_heap[i] = 0;
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
		printf("Heap: Incorrect number of pools");
		correct = false;
	}

	size_t pool_begin, pool_end;
	bool pool_full; 
	uint16_t pool_allocator; 
	for(size_t i = 0; i < block_size_count; i++) {
		pool_begin 	   = pool_controller.pool_begin_indices[i];
		pool_end   	   = pool_controller.pool_end_indices[i];
		pool_full  	   = pool_controller.pool_full[i];
		pool_allocator = pool_controller.pool_allocators[i];

		// Test pool's begin bounds
		if(pool_begin != (i * HEAP_SIZE / block_size_count)) {
			printf("Pool %zu: Incorrect default pool_begin", i);
			correct = false; 
		}

		// Test pool's end bounds
		if(pool_end != ((i+1) * HEAP_SIZE / block_size_count - block_sizes[i])) {
			printf("Pool %zu: Incorrect default pool_end", i);
			correct = false; 
		}

		// Test that pool is empty by default
		if(pool_full) {
			printf("Pool %zu: Incorrect default pool_full", i);
			correct = false; 
		}

		// Verify that pool's allocator refers to beginning of the pool
		if(pool_allocator != pool_begin) {
			printf("Pool %zu: Incorrect default pool_allocator", i);
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
	printf("Starting Test Runner\n\n");

	printf("Tests 1-7: Input verification\n");
	assert(!test_inputs_small_none());
	assert(test_inputs_small_single());
	assert(test_inputs_small_multiple());

	assert(test_inputs_large_single());
	assert(test_inputs_large_multiple());

	assert(test_inputs_mixed_multiple());
	assert(test_inputs_max_single());
	assert(!test_inputs_exceed_single());
	printf("Tests 1-8: PASS\n\n");
	
	printf("Tests 9-14: Pool initialization\n");
	assert(test_pool_init_small_single());
	assert(test_pool_init_small_multiple());

	assert(test_pool_init_large_single());
	assert(test_pool_init_large_multiple());

	assert(test_pool_init_mixed_multiple());
	assert(test_pool_init_max_single());
	printf("Tests 9-14: PASS\n\n");

	printf("Tests 15-20: Pool malloc\n");
	printf("\nBEGIN TEST 15\n");
	assert(test_pool_malloc_single_single());

	printf("\nBEGIN TEST 16\n");
	assert(test_pool_malloc_single_multiple());

	printf("\nBEGIN TEST 17\n");
	assert(test_pool_malloc_multiple_single()); 

	printf("\nBEGIN TEST 18\n");
	assert(test_pool_malloc_multiple_multiple());

	printf("\nBEGIN TEST 19\n");
	assert(test_pool_malloc_max_single());

	printf("\nBEGIN TEST 20\n");
	assert(test_pool_malloc_max_multiple());
	printf("Tests 15-20: PASS\n\n");

	printf("Tests 20-21: Pool free\n");

	printf("\nBEGIN TEST 21\n");
	assert(test_pool_free_single());

	printf("\nBEGIN TEST 22\n");
	assert(test_pool_free_multiple());

	printf("Tests 20-21: PASS\n\n");

	printf("All tests passed\n");
}

/* END TEST RUNNER */


/* BEGIN Heap Inputs Verification Tests */

bool test_inputs_small_none(void) {
	size_t block_sizes[] = {4};
	return verify_heap_inputs(block_sizes, 0);
}

bool test_inputs_small_single(void) {
	size_t block_sizes[] = {4};
	return verify_heap_inputs(block_sizes, 1);
}

bool test_inputs_small_multiple(void) {
	size_t block_sizes[] = {2, 4, 8, 16};
	return verify_heap_inputs(block_sizes, 4);
}

bool test_inputs_large_single(void) {
	size_t block_sizes[] = {65536};
	return verify_heap_inputs(block_sizes, 1);
}

bool test_inputs_large_multiple(void) {
	size_t block_sizes[] = {16384, 32768};
	return verify_heap_inputs(block_sizes, 2);
}

bool test_inputs_mixed_multiple(void) {
	size_t block_sizes[] = {2, 128, 1024, 16384};
	return verify_heap_inputs(block_sizes, 4);
}

bool test_inputs_max_single(void) {
	size_t block_sizes[] = {65536};
	return verify_heap_inputs(block_sizes, 1);
}

bool test_inputs_exceed_single(void) {
	size_t block_sizes[] = {131072};
	return verify_heap_inputs(block_sizes, 1);
}

/* END Heap Inputs Verification Tests */


/* BEGIN Pool Initialization Tests */

bool pool_init_base(const size_t* block_sizes, size_t block_size_count) {
	bool pass = true; 

	pool_deinit(); // Zero global static heap object

	// Force correctness to 0 if any of the preliminary check fails
	pass &= pool_init(block_sizes, block_size_count);
	pass &= verify_heap_init_base(block_sizes, block_size_count);

	return pass;
}

bool test_pool_init_small_single(void) {
	size_t block_sizes[] = {4};
	return pool_init_base(block_sizes, 1);
}

bool test_pool_init_small_multiple(void) {
	size_t block_sizes[] = {2, 4, 8, 16};
	return pool_init_base(block_sizes, 4);
}

bool test_pool_init_large_single(void) {
	size_t block_sizes[] = {32768};
	return pool_init_base(block_sizes, 1);
}

bool test_pool_init_large_multiple(void) {
	size_t block_sizes[] = {32768, 16384};
	return pool_init_base(block_sizes, 2);
}

bool test_pool_init_mixed_multiple(void) {
	size_t block_sizes[] = {2, 128, 1024, 16384};
	return pool_init_base(block_sizes, 4);
}

bool test_pool_init_max_single(void) {
	size_t block_sizes[] = {65536};
	return pool_init_base(block_sizes, 1);
}

/* END Pool Initialization Tests */


/* BEGIN Pool Malloc Tests */

bool test_pool_malloc_single_single(void) {
	bool pass = true; 

	size_t block_sizes[] = {8};
	pass &= pool_init_base(block_sizes, 1);

	uint8_t* ptr = pool_malloc(8);

	// Ensure allocation at correct address
	if(ptr != &g_pool_heap[0]) {
		pass = false; 
	}

	// Ensure pool allocator was properly bumped
	if(pool_controller.pool_allocators[0] != 8) {
		pass = false; 
	}

	return pass; 
}

bool test_pool_malloc_single_multiple(void) {
	bool pass = true; 

	size_t block_sizes[] = {8, 16384};
	pass &= pool_init_base(block_sizes, 2);

	uint8_t* ptr1 = pool_malloc(4);
	uint8_t* ptr2 = pool_malloc(9);

	// Ensure allocation at correct address
	if((ptr1 != &g_pool_heap[0]) || (ptr2 != &g_pool_heap[HEAP_SIZE/2])) {
		pass = false;
	} 

	// Ensure pool allocators were properly bumped
	uint16_t pa1 = pool_controller.pool_allocators[0];
	uint16_t pa2 = pool_controller.pool_allocators[1];
	if((pa1 != 8) || (pa2 != HEAP_SIZE/2 + 16384)) {
		pass = false;
	}
	return pass; 
}

bool test_pool_malloc_multiple_single(void) {
	bool pass = true; 

	size_t block_sizes[] = {4};
	pass &= pool_init_base(block_sizes, 1);

	uint8_t* ptr1 = pool_malloc(4);
	uint8_t* ptr2 = pool_malloc(4);
	uint8_t* ptr3 = pool_malloc(4);
	uint8_t* ptr4 = pool_malloc(4);
	
	// Ensure allocation at correct address
	if((ptr1 != &g_pool_heap[4*0]) || (ptr2 != &g_pool_heap[4*1]) ||
	   (ptr3 != &g_pool_heap[4*2]) || (ptr4 != &g_pool_heap[4*3])) {
		   pass = false;
	}

	// Ensure pool allocator was properly bumped
	if(pool_controller.pool_allocators[0] != 4*4) {
		pass = false; 
	}
	return pass; 
}

bool test_pool_malloc_multiple_multiple(void) {
	bool pass = true; 

	size_t block_sizes[] = {8, 1024};
	pass &= pool_init_base(block_sizes, 2);

	uint8_t* pool0_ptr1 = pool_malloc(8);
	uint8_t* pool0_ptr2 = pool_malloc(8);
	uint8_t* pool1_ptr1 = pool_malloc(1024);
	uint8_t* pool1_ptr2 = pool_malloc(1024);

	// Ensure allocation at correct address
	if((pool0_ptr1 != &g_pool_heap[8*0]) || (pool0_ptr2 != &g_pool_heap[8*1])) {
		pass = false;
	}
	if((pool1_ptr1 != &g_pool_heap[HEAP_SIZE/2 + 1024*0]) ||
	   (pool1_ptr2 != &g_pool_heap[HEAP_SIZE/2 + 1024*1])) {
		   pass = false;
	}

	// Ensure pool allocators were properly bumped
	uint16_t pa1 = pool_controller.pool_allocators[0];
	uint16_t pa2 = pool_controller.pool_allocators[1];
	if((pa1 != 8*2) || (pa2 != HEAP_SIZE/2 + 1024*2)) {
		pass = false;
	}

	return pass; 

}
bool test_pool_malloc_max_single(void) {
	bool pass = true; 

	size_t block_sizes[] = {16384};
	pass &= pool_init_base(block_sizes, 1);

	uint8_t* ptr1 = pool_malloc(16384);
	uint8_t* ptr2 = pool_malloc(16384);
	uint8_t* ptr3 = pool_malloc(16384);
	uint8_t* ptr4 = pool_malloc(16384);

	// Exceed max to ensure it does not corrupt heap
	uint8_t* ptr5 = pool_malloc(16384); 

	// Check that allocation pointer are bumped back to pool beginning
	if(pool_controller.pool_allocators[0] != 0) {
		pass = false;
	}
	return pass;
}

bool test_pool_malloc_max_multiple(void) {
	bool pass = true; 

	size_t block_sizes[] = {16384, 32768};
	pass &= pool_init_base(block_sizes, 2);

	uint8_t* pool0_ptr1 = pool_malloc(16384);
	uint8_t* pool0_ptr2 = pool_malloc(16384);
	uint8_t* pool0_ptr3 = pool_malloc(16384); // Exceed storage limit

	uint8_t* pool1_ptr1 = pool_malloc(32768);
	uint8_t* pool1_ptr2 = pool_malloc(32768); // Exceed storage limit

	// Check that allocation pointer are bumped back to pool beginning
	uint16_t pa1 = pool_controller.pool_allocators[0];
	uint16_t pa2 = pool_controller.pool_allocators[1];
	if((pa1 != 0) || (pa2 != HEAP_SIZE/2)) { 
		pass = false;
	}
	return pass;
}

/* END Pool Malloc Tests */


/* BEGIN Pool Free Tests */

bool test_pool_free_single() {
	bool pass = true; 

	size_t block_sizes[] = {8};
	pass &= pool_init_base(block_sizes, 1);

	uint8_t* ptr1 = pool_malloc(8);
	uint8_t* ptr2 = pool_malloc(8);
	uint8_t* ptr3 = pool_malloc(8);
	uint8_t* ptr4 = pool_malloc(8);

	// Test freeing and reallocating first block
	pool_free(ptr1); 
	ptr1 = pool_malloc(8); 
	if(ptr1 != &g_pool_heap[0]) {
		pass = false; 
	}

	// Test allocating a fifth block
	uint8_t* ptr5 = pool_malloc(8);
	if(ptr5 != &g_pool_heap[8*4]) {
		pass = false; 
	}

	// Test freeing all blocks in order
	pool_free(ptr5);
	pool_free(ptr4);
	pool_free(ptr3);
	pool_free(ptr2);
	pool_free(ptr1);
	if(pool_controller.pool_allocators[0] != 0) {
		pass = false; 
	}

	// Test reallocating and freeing all in reverse order
	ptr1 = pool_malloc(8);
	ptr2 = pool_malloc(8);
	ptr3 = pool_malloc(8);
	ptr4 = pool_malloc(8);
	ptr5 = pool_malloc(8);
	pool_free(ptr1);
	pool_free(ptr2);
	pool_free(ptr3);
	pool_free(ptr4);
	pool_free(ptr5);

	// Allocator should be at the index of the last block freed
	if(pool_controller.pool_allocators[0] != 8*4) {
		pass = false;
	}

	return pass; 
}
bool test_pool_free_multiple() {
	bool pass = true; 

	size_t block_sizes[] = {8, 16384};
	pass &= pool_init_base(block_sizes, 2);

	uint8_t* pool0_ptr1 = pool_malloc(8);
	uint8_t* pool0_ptr2 = pool_malloc(8);
	uint8_t* pool0_ptr3 = pool_malloc(8);
	uint8_t* pool0_ptr4 = pool_malloc(8);

	uint8_t* pool1_ptr1 = pool_malloc(16384);
	uint8_t* pool1_ptr2 = pool_malloc(16384);

	// Test freeing from pool 1
	pool_free(pool1_ptr2);
	if(pool_controller.pool_allocators[1] != HEAP_SIZE/2 + 16384) {
		pass = false; 
	}

	// Test freeing from pool 0, followed by an allocation
	pool_free(pool0_ptr2);
	if(pool_controller.pool_allocators[0] != 8) {
		pass = false; 
	}
	pool0_ptr2 = pool_malloc(8);
	if(pool_controller.pool_allocators[0] != 32) {
		pass = false; 
	}

	return pass; 
}

/* END Pool Free Tests */
