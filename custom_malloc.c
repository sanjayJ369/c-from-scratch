#include <unistd.h>

/*
------ *int brk(void *addr); ------
brk() sets the end of the data segment to the value specified by addr,
when that value is reasonable, the system has enough memory,
and the process does not exceed its maximum data size.

On success, brk() returns zero.
On error, -1 is returned, and errno is set to ENOMEM.


------ void *sbrk(intptr_t increment); ------
sbrk() increments the program's data space by increment bytes.
Calling sbrk() with an increment of 0 can be used to find the current location of the program break.

On success, sbrk() returns the previous program break.
(If the break was increased, then this value is a pointer to the start of the newly allocated memory).
On error, (void *) -1 is returned, and errno is set to ENOMEM.
*/


typedef struct header {
    // size is equal the size of the block (including header)
    size_t size; // the unit of size is the sizeof(header),
    struct header *next;
} header_t;

const size_t CHUNK_SIZE = 1024 * 10; // in bytes
const size_t MIN_BLOCK_SIZE = 2;

header_t *head = NULL; // free list

void *my_malloc(size_t size); // returns a free block
void my_free(void *ptr); // frees a block
void *morecore(size_t size); // asks OS for some memory
int refill_freelist(); // adds block of size CHUNK_SIZE to the free list

// splits the block and returns pointer to the right block, handles pointers
// here size is in the units of header_t
header_t *split_block(header_t *block, size_t size);

header_t *split_block(header_t *block, size_t size) {
    header_t *right = block + size;
    right->size = block->size - size;
    right->next = block->next;

    block->size = size;
    block->next = right;

    return right;
}

int refill_freelist() {
    void *block = sbrk(CHUNK_SIZE);
    if (block == (void *) -1) {
        return -1; // failed to allocate memory
    }

    header_t *new_block = block;
    new_block->size = CHUNK_SIZE / sizeof(header_t);
    my_free(new_block + 1);
    return 0;
}

void *morecore(size_t size) {
    size_t req = size * sizeof(header_t);
    return sbrk(req);
}

void my_free(void *ptr) {
    header_t *block = (header_t *) ptr - 1;
    header_t *curr = head;
    header_t *prev = NULL;

    while (curr != NULL && curr < block) {
        prev = curr;
        curr = curr->next;
    }

    // free list is empty
    if (prev == NULL && curr == NULL) {
        head = block;
        return;
    }

    // insertion at the start
    if (prev == NULL) {
        // check for right coalescing
        if (block + block->size == curr) {
            block->size += curr->size;
            block->next = curr->next;
        } else {
            block->next = curr;
        }
        head = block;
        return;
    }

    // merge neither (default)
    prev->next = block;
    block->next = curr;


    // merge left
    if (prev + prev->size == block) {
        prev->size += block->size;
        prev->next = curr;
        block = prev; // takes care for merge on right
    }

    // merge right
    if (block + block->size == curr) {
        block->size += curr->size;
        block->next = curr->next;
    }
}

void *my_malloc(size_t size) {
    size_t req = ((size + sizeof(header_t) - 1) / sizeof(header_t)) + 1;
    if (head == NULL) {
        if (refill_freelist() == -1) {
            return NULL;
        }
    }

    header_t *curr = head;
    header_t *prev = NULL;

    while (curr != NULL) {
        if (curr->size >= req) {
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) {
        header_t *new_block = morecore(req);
        if (new_block == NULL) {
            return NULL;
        }
        new_block->size = req;
        return (void *) (new_block + 1);
    }

    // check should split
    if (curr->size >= req + MIN_BLOCK_SIZE) {
        split_block(curr, req);
    }

    if (prev == NULL) {
        head = head->next;
    } else {
        prev->next = curr->next;
    }

    return (void *) (curr + 1);
}
