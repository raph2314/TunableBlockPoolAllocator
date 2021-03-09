#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifndef POOL_ALLOC_H
#define POOL_ALLOC_H

#define HEAP_SIZE 	65536
#define MAX_POOLS	16


/* Pool Controller
 *
 * Used as a global object to track the state of the heap. 
 * Its size is limited by the MAX_POOLS assumpion described in
 * the pool_init() docstring
 *
*/
typedef struct {
	uint8_t num_pools;				// Used to track heap parameters
	size_t block_sizes[MAX_POOLS];  // Used to track heap parameters
	
	uint16_t pool_begin_indices[MAX_POOLS];
	uint32_t pool_end_indices[MAX_POOLS];
	
	bool pool_full[MAX_POOLS]; 
	uint16_t pool_allocators[MAX_POOLS];  // Holds pool allocator idx in g_pool_heap
} pool_controller_t; 


static uint8_t g_pool_heap[HEAP_SIZE];
static pool_controller_t pool_controller; 


/* DEBUG function for printing the heap at <idx +/- padding>
*/
void print_heap_range(size_t idx, size_t padding);


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
 * Implementation Assumptions:
 *
 * 1. The heap is evenly divided amongst pools.
 *
 * 2. Block sizes cannot exceed pool size
 *
 * Input Assumptions:
 
 * 1. Provided block sizes are powers of 2.
 *
 * 2. Block sizes are provided in ascending order
 *
 * 3. block_size_count must be a power of 2, up to 2^4.
 *    This is because splitting the heap into 16 pools 
 *    of 4096 bytes allows for 13 unique block sizes
 *    from 2^0 to 2^12.
 *    
 *    If 2^5 blocks_size_counts were allowed, the heap
 *    would contain 32 pools of 2048 bytes, but only 
 *    12 unique block size from 2^0 to 2^11. 
 *
 *    The application could of course call for repeating
 *    block sizes, but for the sake of simplifcation, 
 *    only unique block sizes are used. 
 *    	
 *    This simplifying assumption saves pre-allocated 
 *    memory by limiting the size of the arrays in 
 *    the Pool Controller.
 *
*/
bool pool_init(const size_t* block_sizes, size_t block_size_count);


/* Allocate n bytes.
 *
 * Returns pointer to allocated memory on success, NULL on failure.
*/ 
void* pool_malloc(size_t n);


/* Release allocation pointed to by ptr.
 *
 * Assumptions:
 * 1. Argument must match a pointer earlier returned by a call
 *    to pool_malloc()
 *
 *    This assumption is justified since it mimics the behaviour 
 *	  of malloc() and free() 
 *	  (ref: https://stackoverflow.com/questions/5308758/can-a-call-to-free-in-c-ever-fail)
 *
 *
 */ 
void pool_free(void* ptr);


#endif // POOL_ALLOC_H