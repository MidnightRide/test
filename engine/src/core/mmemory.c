#include "mmemory.h"

#include "core/logger.h"
#include "core/mstring.h"
#include "platform/platform.h"

// TODO(satvik): custom string lib
#include <string.h>
#include <stdio.h>

struct memory_stats
{
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
} memory_stats;

typedef struct memory_system_state
{
    struct memory_stats stats;
    u64 alloc_count;
} memory_system_state;

static memory_system_state *state_ptr;

static const char *memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
    "LINEAR_ALLC",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      ",
};

void memory_system_initialise(u64 *memory_requirements, void *state)
{
    *memory_requirements = sizeof(memory_system_state);
    if (state == 0)
        return;
    
    state_ptr = state;
    state_ptr->alloc_count = 0;
    platform_zero_memory(&state_ptr->stats, sizeof(state_ptr->stats));
}

void memory_system_shutdown(void *state)
{
    state_ptr = 0;
}

void *mallocate(u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN)
    {
        MWARN("mallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }
    
    if (state_ptr)
    {
        state_ptr->stats.total_allocated += size;
        state_ptr->stats.tagged_allocations[tag] += size;
        state_ptr->alloc_count++;
    }
    
    // TODO(satvik): memory alignment.
    void *block = platform_allocate(size, false);
    platform_zero_memory(block, size);
    return block;
}

void mfree(void *block, u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN)
    {
        MWARN("mfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }
    
    if (state_ptr)
    {
        state_ptr->stats.total_allocated -= size;
        state_ptr->stats.tagged_allocations[tag] -= size;
    }
    
    // TODO(satvik): memory alignment.
    platform_free(block, false);
}

void *mzero_memory(void *block, u64 size)
{
    return platform_zero_memory(block, size);
}

void *mcopy_memory(void *dest, const void *source, u64 size)
{
    return platform_copy_memory(dest, source, size);
}

void *mset_memory(void *dest, i32 value, u64 size)
{
    return platform_set_memory(dest, value, size);
}

char *get_memory_use_str()
{
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;
    
    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = string_length(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i)
    {
        char unit[4] = "Xib";
        float amount = 1.0f;
        if (state_ptr->stats.tagged_allocations[i] >= gib)
        {
            unit[0] = 'G';
            amount = state_ptr->stats.tagged_allocations[i] / (float)gib;
        }
        else if (state_ptr->stats.tagged_allocations[i] >= mib)
        {
            unit[0] = 'M';
            amount = state_ptr->stats.tagged_allocations[i] / (float)mib;
        }
        else if (state_ptr->stats.tagged_allocations[i] >= kib)
        {
            unit[0] = 'K';
            amount = state_ptr->stats.tagged_allocations[i] / (float)kib;
        }
        else
        {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)state_ptr->stats.tagged_allocations[i];
        }
        
        i32 length = snprintf(buffer + offset, 8000, " %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
        offset += length;
    }
    char *out_string = string_duplicate(buffer);
    return out_string;
}

u64 get_memory_alloc_count()
{
    if (state_ptr)
        return state_ptr->alloc_count;
    
    return 0;
}