#ifndef _BINARY_ALLOCATOR_H
#define _BINARY_ALLOCATOR_H

#include "shared.hpp"
#include <assert.h>


typedef struct {
    block *blocks;
    forward_memory **free_blocks; // ith element is a forward-list of memory with capacity 2^i
    uint64_t max_order;
} binary_allocator;

/*
free_blocks:
capacity = 2^0: [*, ->][*, ->]0
capacity = 2^1: [*, ->]0
*/

binary_allocator* bin_alloc_create(uint64_t byte_count);
binary_allocator* bin_alloc_create_with_block_size(uint64_t block_count, uint64_t block_size);
void bin_alloc_destroy(binary_allocator *ba);

void* bin_alloc_allocate(binary_allocator *ba, uint64_t bytes_needed);
uint64_t bin_alloc_deallocate(binary_allocator *ba, void *memory);

void bin_alloc_print(binary_allocator ba);


void recursive_free_forward_memory(forward_memory *block) {
    if (block == NULL) return;

    recursive_free_forward_memory(block->next);
    free(block);
}

binary_allocator* bin_alloc_create(uint64_t byte_count) {
    uint64_t max_order = closest_n_pow2(byte_count);
    byte_count = pow2(max_order);


    void *memory = (void*) calloc(byte_count, 1);
    assert(memory != nullptr);


    auto *blocks = (block*) calloc(byte_count, sizeof(block));
    assert(blocks != nullptr);

    for (uint64_t i = 0; i < byte_count; ++i) {
        blocks[i].memory = (char*)memory + i;
        blocks[i].taken = 0;
    }


    auto **free_blocks = (forward_memory**) calloc(max_order + 1, sizeof(forward_memory*));
    assert(free_blocks != nullptr);

    free_blocks[max_order] = static_cast<forward_memory *>(calloc(1, sizeof(forward_memory)));
    assert(free_blocks[max_order] != NULL);
    free_blocks[max_order]->memory = memory;

    auto* ba = static_cast<binary_allocator *>(calloc(sizeof(binary_allocator), 1));

    ba->blocks = blocks;
    ba->free_blocks = free_blocks;
    ba->max_order = max_order;

    return ba;
}

binary_allocator* bin_alloc_create_with_block_size(uint64_t block_count, uint64_t block_size) {
    return bin_alloc_create(block_count * block_size);
}

void bin_alloc_destroy(binary_allocator *ba) {
    free(ba->blocks[0].memory);
    free(ba->blocks);

    for (uint64_t i = 0; i <= ba->max_order; ++i) {
        recursive_free_forward_memory(ba->free_blocks[i]);
    }
    free(ba->free_blocks);

    free(ba);
}

void* bin_alloc_divide_block(binary_allocator *ba, uint64_t order, uint64_t bytes_needed) {
    if (order == 0 || order > ba->max_order) return nullptr;
    void *result = nullptr;

    void *memory1 = ba->free_blocks[order]->memory;
    void *memory2 = (char*)ba->free_blocks[order]->memory + pow2(order - 1);

    uint64_t block_index = 0;
    for (; block_index < pow2(ba->max_order); ++block_index) if (ba->blocks[block_index].memory == memory1) break;

    forward_memory *next = ba->free_blocks[order]->next;
    free(ba->free_blocks[order]);
    ba->free_blocks[order] = next;

    forward_memory *leftmost_free = ba->free_blocks[order - 1];

    auto *new_block2 = static_cast<forward_memory *>(calloc(1, sizeof(forward_memory)));
    new_block2->memory = memory2;
    new_block2->next = leftmost_free;

    auto *new_block1 = static_cast<forward_memory *>(calloc(1, sizeof(forward_memory)));
    new_block1->memory = memory1;
    new_block1->next = new_block2;

    ba->free_blocks[order - 1] = new_block1;

    if (pow2(order - 1) > bytes_needed) {
        result = bin_alloc_divide_block(ba, order - 1, bytes_needed);
    } else {
        result = memory1;

        ba->blocks[block_index].taken = bytes_needed;
        free(ba->free_blocks[order - 1]);
        ba->free_blocks[order - 1] = new_block2;
    }

    return result;
}

void* bin_alloc_allocate(binary_allocator *ba, uint64_t bytes_needed) {
    bytes_needed = closest_pow2(bytes_needed);
    uint64_t order = closest_n_pow2(bytes_needed);

    forward_memory *current_fm = ba->free_blocks[order];

    if (current_fm != nullptr) {
        uint64_t block_index = 0;
        for (; block_index < pow2(ba->max_order); ++block_index) if (ba->blocks[block_index].memory == current_fm->memory) break;
        ba->blocks[block_index].taken = bytes_needed;

        forward_memory *next = current_fm->next;
        void *memory = current_fm->memory;

        ba->free_blocks[order] = next;

        free(current_fm);

        return memory;
    }

    while (order <= ba->max_order && ba->free_blocks[order] == nullptr) ++order;
    if (order > ba->max_order) return nullptr;

    return bin_alloc_divide_block(ba, order, bytes_needed);
}

uint64_t bin_alloc_deallocate(binary_allocator *ba, void *memory) {
    uint64_t block_index = 0;
    for (; block_index < pow2(ba->max_order); ++block_index) if (ba->blocks[block_index].memory == memory) break;

    uint64_t order = closest_n_pow2(ba->blocks[block_index].taken);
    uint64_t result = ba->blocks[block_index].taken;

    ba->blocks[block_index].taken = 0;

    forward_memory *leftmost_free = ba->free_blocks[order];

    forward_memory *new_block = static_cast<forward_memory *>(calloc(1, sizeof(forward_memory)));
    new_block->memory = memory;
    new_block->next = leftmost_free;
    ba->free_blocks[order] = new_block;

    return result;
}

void bin_alloc_print(binary_allocator ba) {
    printf(
            "ba = {\n"
    );

    if (ba.max_order < 6) {
        printf("\tblocks = {\n\t\t");
        for (uint64_t i = 0; i < pow2(ba.max_order); ++i) {
            printf("%p, %" PRIu64"; ", ba.blocks[i].memory, ba.blocks[i].taken);
        }
        printf("\n\t}\n");
    }


    printf("\tfree_blocks = {\n");
    for (uint64_t i = 0; i <= ba.max_order; ++i) {
        forward_memory *rightmost_free = ba.free_blocks[i];

        printf("\t\t");
        if (rightmost_free == nullptr) {
            printf("(nil); ");
        } else {
            while (rightmost_free != nullptr) {
                printf("%p; ", rightmost_free->memory);
                rightmost_free = rightmost_free->next;
            }
        }
        printf("\b\b \b\n");
    }
    printf("\t}\n");


    printf("\tmax_order = %" PRIu64"\n", ba.max_order);

    printf("}\n");
}

#endif // _BINARY_ALLOCATOR_H