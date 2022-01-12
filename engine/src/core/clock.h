#pragma once

#include "defines.h"

typedef struct clock
{
    f64 start_time;
    f64 elapsed;
} clock;

// Updates the provided clock. Should be called just before checking elapsed time.
// Has no effect on non-started clocks.
MAPI void clock_update(clock *clock);

// Starts the provided clock. Resets elapsed time.
MAPI void clock_start(clock *clock);

// Stops the provided clock. Does not reset elapsed time.
MAPI void clock_stop(clock *clock);