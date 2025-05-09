#pragma once
#include <stdlib.h>

typedef struct Arena {
    void* data;
    size_t offset;
    size_t length;
} Arena;

Arena* arena_new(size_t data_length);
void arena_free_all(Arena* arena);
void* arena_alloc(Arena* arena, size_t size);
void arena_destroy(Arena* arena);
