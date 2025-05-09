#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <cstdio>
#include <cstdlib>

#include "arena.hpp"


// Arena* g_temp_allocator;

Arena* arena_new(size_t data_length) {
    auto arena = (Arena*)(malloc(sizeof(Arena)));
    
    arena->offset = arena->length = data_length;
    return arena;
}

void arena_free_all(Arena* arena) { arena->offset = 0; }

// Just double the underlying data length.
// void arena_resize(Arena* arena) { realloc(arena->data, arena->length * 3); }

void* arena_alloc(Arena* arena, size_t size) {
    // Check to see if the backing memory has space left
    assert(arena->offset + size <= arena->length);

    // Can probably just automatically resize if neccessary.
    // However, that will probably fuck up pointers that point
    // into some part of the arena.

    void* ptr = &arena->data + arena->offset;
    arena->offset += size;
    // Zero new memory by default
    memset(ptr, 0, size);
    return ptr;
}

void arena_destroy(Arena* arena) {
    assert(arena != NULL && arena->data != NULL);
    free(arena->data);
    free(arena);
}