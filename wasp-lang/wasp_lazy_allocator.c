#include "wasp_lazy_allocator.h"

bool wasp_add_lazy_allocator_block(wasp_lallocator *alloc, size_t size)
{
    wasp_lallocator_block *block = (wasp_lallocator_block *)malloc(sizeof(wasp_lallocator_block) + size);
    if (block != NULL)
    {
        block->available = size;
        block->next_block = alloc->block;
        alloc->block = block;

        return true;
    }
    else
        return false;
}

bool wasp_create_lazy_allocator(wasp_lallocator *alloc)
{
    assert(alloc != NULL);

    alloc->block = NULL;
    return wasp_add_lazy_allocator_block(alloc, wasp_lallocator_block_SIZE);
}

void *wasp_lazy_alloc(wasp_lallocator *alloc, size_t size)
{
    assert(alloc != NULL);

    if (size > alloc->block->available)
    {
        if (size <= wasp_lallocator_block_SIZE)
        {
            if (!wasp_add_lazy_allocator_block(alloc, wasp_lallocator_block_SIZE))
                return NULL;
        }
        else
        {
            // Insert block for this allocation only
            wasp_lallocator_block *last_block = alloc->block;
            alloc->block = last_block->next_block;

            if (!wasp_add_lazy_allocator_block(alloc, size))
            {
                // Fix allocator
                alloc->block = last_block;
                return NULL;
            }

            wasp_lallocator_block *new_block = alloc->block;

            // Fix allocator
            last_block->next_block = new_block;
            alloc->block = last_block;

            new_block->available = 0;
            return (void *)alloc->block + sizeof(wasp_lallocator_block);
        }
    }

    alloc->block->available -= size;
    return (void *)alloc->block + sizeof(wasp_lallocator_block) + alloc->block->available;
}
void *wasp_lazy_alloc_array(wasp_lallocator *alloc, size_t size, size_t count)
{
    return wasp_lazy_alloc(alloc, size * count);
}

void wasp_destroy_lazy_allocator(wasp_lallocator *alloc)
{
    assert(alloc != NULL);

    // Free every block
    for (wasp_lallocator_block *block = alloc->block, *next_block = NULL; block != NULL; block = next_block)
    {
        next_block = block->next_block;
        free(block);
    }

    alloc->block = NULL;
}