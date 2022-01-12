#include "core/event.h"

#include "core/mmemory.h"
#include "containers/darray.h"

typedef struct registered_event
{
    void *listener;
    PFN_on_event callback;
} registered_event;

typedef struct event_code_entry
{
    registered_event *events;
} event_code_entry;

// This should be more than enough codes...
#define MAX_MESSAGE_CODES 16384

// State structure.
typedef struct event_system_state
{
    // Lookup table for event codes.
    event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;

/**
 * Event system internal state
 */
static event_system_state *state_ptr;

void event_system_initialise(u64 *memory_requirement, void *state)
{
    *memory_requirement = sizeof(event_system_state);
    if (state == 0)
        return;
    
    mzero_memory(state, sizeof(state));
    state_ptr = state;
}

void event_system_shutdown(void *state)
{
    if (state_ptr)
    {
        // Free the events arrays. Any objects  pointed to should be destroyed on their own.
        for (u16 i = 0; i < MAX_MESSAGE_CODES; i++)
        {
            if (state_ptr->registered[i].events != 0)
            {
                darray_destroy(state_ptr->registered[i].events);
                state_ptr->registered[i].events = 0;
            }
        }
    }
    state_ptr = 0;
}

b8 event_register(u16 code, void *listener, PFN_on_event on_event)
{
    if (!state_ptr)
        return false;
    
    if (state_ptr->registered[code].events == 0)
        state_ptr->registered[code].events = darray_create(registered_event);
    
    u64 registered_count = darray_length(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; i++)
    {
        if (state_ptr->registered[code].events[i].listener == listener)
        {
            // TODO(satvik): warn
            return false;
        }
    }
    
    // If at this point, no duplicate was found, proceed with registration.
    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    darray_push(state_ptr->registered[code].events, event);
    
    return true;
}

b8 event_unregister(u16 code, void *listener, PFN_on_event on_event)
{
    if (!state_ptr)
        return false;
    
    // On nothing is registered for the code, boot out.
    if (state_ptr->registered[code].events == 0)
    {
        // TODO(satvik): warn
        return false;
    }
    
    u64 registered_count = darray_length(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i)
    {
        registered_event e = state_ptr->registered[code].events[i];
        if (e.listener == listener && e.callback == on_event)
        {
            // Found one, remove it
            registered_event popped_event;
            _darray_pop_at(state_ptr->registered[code].events, i, &popped_event);
            return true;
        }
    }
    
    // Not found.
    return false;
}

b8 event_fire(u16 code, void *sender, event_context data)
{
    if (!state_ptr)
        return false;
    
    // If nothing is registered for the code, boot out.
    if (state_ptr->registered[code].events == 0)
        return false;
    
    u64 registered_count = darray_length(state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; ++i)
    {
        registered_event e = state_ptr->registered[code].events[i];
        if (e.callback(code, sender, e.listener, data))
        {
            // Message has been handled, do not send to other listeners.
            return true;
        }
    }
    
    // Not found.
    return false;
}
