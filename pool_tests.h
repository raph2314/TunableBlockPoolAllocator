#include "pool_alloc.h"

#ifndef POOL_TESTS_H
#define POOL_ALLOC_H

/* 
 * NOTE: Tests that do not satisfy input assumptions are omitted
*/ 	


/* TEST HELPERS
 *
 * Complement test functionality
*/
// Setup for reinitializing global objects between tests
void pool_deinit(void); 

// Verify overall default heap correctness
bool verify_heap_init_base(const size_t* block_sizes, size_t block_size_count);

// Verify default pool controller struct 
bool verify_heap_init_pool_controller(const size_t* block_sizes, size_t block_size_count);

// Check default references between blocks across g_pool_heap
bool verify_heap_init_block_references(const size_t* block_sizes, size_t block_size_count);


/* TEST RUNNER
 *
 * Manually runs all existing tests
 */
void TestRunner(void);


/* Heap Inputs Verification Tests
 *
 * Naming convention:
 * test_inputs_<block_sizes>_<num_pools>()
*/
bool test_inputs_small_none(void);
bool test_inputs_small_single(void);
bool test_inputs_small_multiple(void);
bool test_inputs_large_single(void);
bool test_inputs_large_multiple(void);
bool test_inputs_mixed_multiple(void);
bool test_inputs_max_single(void);
bool test_inputs_exceed_single(void);


/* Pool Initialization Tests
 *
 * Naming convention:
 * test_pool_init_<block_sizes>_<num_pools>()
*/
bool pool_init_base(const size_t* block_sizes, size_t block_size_count);
bool test_pool_init_small_single(void);
bool test_pool_init_small_multiple(void);
bool test_pool_init_large_single(void);
bool test_pool_init_large_multiple(void);
bool test_pool_init_mixed_multiple(void);
bool test_pool_init_max_single(void);


/* Pool Malloc Tests
 *
 * Naming convention:
 * test_pool_malloc_<num_mallocs/pool>_<num_pools>()
*/
bool test_pool_malloc_single_single(void);
bool test_pool_malloc_single_multiple(void);
bool test_pool_malloc_multiple_single(void);
bool test_pool_malloc_multiple_multiple(void);
bool test_pool_malloc_max_single(void);
bool test_pool_malloc_max_multiple(void);


/* Pool Free Tests
 *
 * Naming convention:
 * test_pool_free_<num_pools>()
*/
bool test_pool_free_single();
bool test_pool_free_multiple(); 

#endif // POOL_TESTS_H