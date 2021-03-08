#include "pool_alloc.h"

/* 
 * NOTE: Tests that do not satisfy input assumptions are omitted
*/ 	


/* TEST HELPERS
 *
 * Complement test functionality
*/
void pool_deinit(void); // Setup for reinitializing global objects between tests

bool verify_heap_init_base(const size_t* block_sizes, size_t block_size_count);

// Verify pool controller struct 
bool verify_heap_init_pool_controller(const size_t* block_sizes, size_t block_size_count);

// Check references between block across g_pool_heap
bool verify_heap_init_block_references(const size_t* block_sizes, size_t block_size_count);


/* TEST RUNNER
 *
 * Manually runs all existing tests
 */
void TestRunner(void);


/* Heap Inputs Verification Tests
 *
 * Naming convention:
 * verify_inputs_<block_sizes>_<num_blocks>()
 *
*/
bool verify_inputs_small_none(void);
bool verify_inputs_small_single(void);
bool verify_inputs_small_multiple(void);
bool verify_inputs_large_single(void);
bool verify_inputs_large_multiple(void);
bool verify_inputs_mixed_multiple(void);
bool verify_inputs_max_single(void);
bool verify_inputs_exceed_single(void);


/* Pool Initialization Tests
 *
 * Naming convention:
 * pool_init_<block_sizes>_<num_blocks>()
*/
bool pool_init_base(const size_t* block_sizes, size_t block_size_count);
bool pool_init_small_single(void);
bool pool_init_small_multiple(void);
bool pool_init_large_single(void);
bool pool_init_large_multiple(void);
bool pool_init_mixed_multiple(void);
bool pool_init_max_single(void);