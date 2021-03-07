#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifndef POOL_ALLOC_H
#define POOL_ALLOC_H

#define HEAP_SIZE 	65536
#define MAX_POOLS	16



// TODO: Document struct
typedef struct {
	uint16_t pool_begin_indices[MAX_POOLS];
	uint16_t pool_end_indices[MAX_POOLS];
	uint8_t* ppool_allocators[MAX_POOLS];
} pool_controller_t; 


/* Helper function to validate inputs for heap configuration
 *
 * Returns true for valid inputs, false otherwise
 */
bool verify_heap_inputs(const size_t* block_sizes, size_t block_size_count);

/* Initialize the pool allocator with a set of block sizes appropriate
 * for this application.
 *
 * Returns true on success, false on failure.
 *
 * Assumptions:
 *
 * 1. The heap is evenly divided amongst pools.
 *
 * 2. Provided block sizes are powers of 2.
 *
 * 3. Provided block sizes must be less than the size
 *    of the pools.
 *
 * 4. block_size_count must be a power of 2, up to 2^4.
 *    This is because splitting the heap into 16 pools 
 *    of 4096 bytes allows for 13 unique block sizes
 *    from 2^0 to 2^12.
 *    
 *    If 2^5 blocks_size_counts were allowed, the heap
 *    would contain 32 pools of (2048 bytes), but only 
 *    12 unique block size from 2^0 to 2^11. 
 *
 *    The application could of course call for repeating
 *    block sizes, but for the sake of simplifcation, 
 *    only unique block sizes are used. 
 *    	
 *    This simplifying assumtion saves pre-allocated 
 *    memory by limiting the size of the arrays in 
 *    the Pool Controller.
 *
 * Time complexity: O(HEAP_SIZE)  --> Double check
*/
bool pool_init(const size_t* block_sizes, size_t block_size_count);

/* Allocate n bytes.
 *
 * Returns pointer to allocate memory on success, NULL on failure.
*/ 
void* pool_malloc(size_t n);

// Release allocation pointed to by ptr.
void pool_free(void* ptr);

#endif // POOL_ALLOC_H