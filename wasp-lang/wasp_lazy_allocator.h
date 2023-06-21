#pragma once
#include "wasp_common.h"

#define wasp_lallocator_block_SIZE 2048

typedef struct wasp_lallocator_block
{
    size_t available;
    struct wasp_lallocator_block *next_block;
} wasp_lallocator_block;

typedef struct wasp_lallocator
{
    wasp_lallocator_block *block;
} wasp_lallocator;

bool wasp_create_lazy_allocator(wasp_lallocator* alloc);

void* wasp_lazy_alloc(wasp_lallocator* alloc, size_t size);
void* wasp_lazy_alloc_array(wasp_lallocator* alloc, size_t size, size_t count);

void wasp_destroy_lazy_allocator(wasp_lallocator* alloc);