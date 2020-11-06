#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>

// MIN/MAX found at https://stackoverflow.com/questions/3437404/min-and-max-in-c
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//Defined for better readability
#define TRUE 1
#define FALSE 0

/* READ:
 * Throughout the code you will see method starting with underscore(_)
 * this is used to enable use of recursive methods without editing the header file declaration
 */


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList {
    // doubly-linked list
    struct memoryList *last;
    struct memoryList *next;

    //unsigned 64bit long
    size_t size;            // How many bytes in this block?
    char allocated;          // 1 if this block is allocated,
    // 0 if this block is free.
    void *ptr;           // location of block in memory pool.
};

strategies myStrategy = NotSet;    // Current strategy


size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *next;


/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

//recursive free
void _free(struct memoryList *mem) {
    if (mem -> next != head) {
        _free(mem -> next);
    }
    free(mem);
}


void initmem(strategies strategy, size_t sz) {
    myStrategy = strategy;

    /* all implementations will need an actual block of memory to use */
    mySize = sz;

    if (myMemory != NULL) {
        free(myMemory); /* in case this is not the first time initmem is called */
        _free(head);
    }
    /* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */

    myMemory = malloc(sz);

    struct memoryList *new = (struct memoryList*) malloc(sizeof(struct memoryList));
    new -> next = new;
    new -> last = new;

    new -> ptr = myMemory;
    new -> size = sz;
    new -> allocated = FALSE;

    head = new;
    next = new;

    /* TODO: Initialize memory management structure. */
}

struct memoryList *worst_free(size_t size) {
    //ptr to the worst fit block of mem
    struct memoryList *worst = NULL;

    //ptr to the head of mem for traversal
    struct memoryList *mem = head;

    do {
        //Compare worst unallocated blocks iteratively
        if (mem -> allocated == FALSE && mem -> size >= size && (worst == NULL || mem -> size > worst -> size)) {
            worst = mem;
        }
        mem = mem -> next;
    } while (mem != head); //recursively check node until we hit the head
    return worst;
}
struct memoryList *split(struct memoryList *mem, size_t size) {
    mem -> allocated = TRUE;
    if (mem -> size != size) {
        struct memoryList *new = (struct memoryList *) malloc(sizeof(struct memoryList));
        new -> size = mem -> size - size;
        mem -> size = size;
        new -> allocated = 0;

        new -> ptr = mem -> ptr + size;

        mem -> next -> last = new;
        new -> next = mem -> next;
        new -> last = mem;
        mem -> next = new;
        return new;
    }
    mem -> size = size;
    return NULL;
}

struct memoryList *_alloc(struct memoryList *mem, size_t size) {
    split(mem, size);
    return mem;
}


void *get_block_ptr(struct memoryList *mem, size_t requested) {
    if (mem == NULL){
        return NULL;
    }
    struct memoryList *allocated = _alloc(mem, requested);
    next = allocated -> next;
    return allocated -> ptr;
}



void *mymalloc(size_t requested) {
    assert((int) myStrategy > 0);

    switch (myStrategy) {
        case NotSet:
            return NULL;
        case First:
            return NULL;
        case Best:
            return NULL;
        case Worst:
            return get_block_ptr(worst_free(requested), requested);
        case Next:
            return NULL;
    }
    return NULL;
}

//Frees a node from the mem linked list and links the next and last node
void free_node(struct memoryList *node) {
    struct memoryList *tmpn = node -> next;
    struct memoryList *tmpl = node -> last;

    tmpn -> last = node -> last;
    tmpl -> next = node -> next;

    tmpl -> size = tmpl -> size + node -> size;
    if (next == node) {
        next = tmpn;
    }
    free(node);
}

//Recursively backtrack through memory to find the start of a free block
struct memoryList *find_start_free_block(struct memoryList *mem) {
    if (mem -> last -> allocated == TRUE || mem == head) {
        return mem;
    }
    return find_start_free_block(mem -> last);
}

//Links the next node with the current one
struct memoryList *link_next(struct memoryList *mem) {
    while (mem -> next -> allocated == FALSE && mem -> next != head) {
        free_node(mem -> next);
    }
    return mem;
}

//I decided it would be best to create another method that takes in the memory as a parameter
//this way I can also call it recursively.
void _myfree(void *block, struct memoryList *mem) {
    if (mem -> ptr == block) {
        if (mem -> allocated == FALSE) {
            printf("Mem is already freed");
        }
        mem -> allocated = FALSE;
        link_next(find_start_free_block(mem));
    } else {
        if (mem -> next == head) {
            printf("Mem block reached its head");
        } else {
            _myfree(block, mem -> next);
        }
    }
}


/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void *block) {
    _myfree(block, head);
    return;
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when we refer to "memory" here, we mean the
 * memory pool this module manages via initmem/mymalloc/myfree.
 */

int _mem_holes(struct memoryList *mem) {
    int total = mem -> next == head ? 0 : _mem_holes(mem -> next);
    return total + (mem -> allocated == FALSE ? 1 : 0);
}

/* Get the number of contiguous areas of free space in memory. */
int mem_holes() {
    return _mem_holes(head);
}

size_t _mem_allocated(struct memoryList *mem) {
    size_t total = mem -> next == head ? 0 : _mem_allocated(mem -> next);
    return total + (mem -> allocated == TRUE ? mem -> size : 0);
}

/* Get the number of bytes allocated */
int mem_allocated() {
    return _mem_allocated(head);
}

size_t _mem_free(struct memoryList *mem) {
    size_t total = mem -> next == head ? 0 : _mem_free(mem -> next);
    return total + (mem -> allocated == FALSE ? mem -> size : 0);
}

/* Number of non-allocated bytes */
int mem_free() {
    return _mem_free(head);
}

size_t _mem_largest_free(struct memoryList *mem) {
    size_t largestfree = mem -> next == head ? 0 : _mem_largest_free(mem -> next);
    return mem -> allocated == FALSE ? MAX(largestfree, mem -> size) : largestfree;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free() {
    return _mem_largest_free(head);
}

int _mem_smallest_free(int size, struct memoryList *mem) {
    return (mem -> next == head ? 0 : _mem_smallest_free(size, mem -> next)) +
           (mem -> allocated == FALSE && mem -> size <= size ? 1 : 0);
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size) {
    return _mem_smallest_free(size, head);
}

char _mem_is_alloc(void *ptr, struct memoryList *mem) {
    if (mem -> ptr == ptr) {
        return mem -> allocated;
    } else {
        if (mem -> next == head) {
            return 0;
        }
        return _mem_is_alloc(ptr, mem -> next);
    }
}

char mem_is_alloc(void *ptr) {
    return _mem_is_alloc(ptr, head);
}

/* 
 * Feel free to use these functions, but do not modify them.  
 * The test code uses them, but you may find them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool() {
    return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total() {
    return mySize;
}


// Get string name for a strategy. 
char *strategy_name(strategies strategy) {
    switch (strategy) {
        case Best:
            return "best";
        case Worst:
            return "worst";
        case First:
            return "first";
        case Next:
            return "next";
        default:
            return "unknown";
    }
}

// Get strategy from name.
strategies strategyFromString(char *strategy) {
    if (!strcmp(strategy, "best")) {
        return Best;
    } else if (!strcmp(strategy, "worst")) {
        return Worst;
    } else if (!strcmp(strategy, "first")) {
        return First;
    } else if (!strcmp(strategy, "next")) {
        return Next;
    } else {
        return 0;
    }
}


/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */


void dump_print_mem(struct memoryList *mem, int i) {
    printf("size: %zu, alloc: %i, index: %i\n", mem -> size, mem -> allocated, i);
    if (mem -> next == head) return;
    dump_print_mem(mem -> next, i + 1);
}

/* Use this function to print out the current contents of memory. */
void print_memory() {
    printf("[[MEM CONTENT>>\n");
    dump_print_mem(head, 0);
}

/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */
void print_memory_status() {
    printf("%d out of %d bytes allocated.\n", mem_allocated(), mem_total());
    printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n", mem_free(), mem_holes(),
           mem_largest_free());
    printf("Average hole size is %f.\n\n", ((float) mem_free()) / mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
    strategies strat;
    void *a, *b, *c, *d, *e;
    if (argc > 1)
        strat = strategyFromString(argv[1]);
    else
        strat = First;


    /* A simple example.
       Each algorithm should produce a different layout. */

    initmem(strat, 500);

    a = mymalloc(100);
    b = mymalloc(100);
    c = mymalloc(100);
    myfree(b);
    d = mymalloc(50);
    myfree(a);
    e = mymalloc(25);

    print_memory();
    print_memory_status();

}
