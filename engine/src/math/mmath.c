#include "mmath.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 rand_seeded = false;

/**
 * Note that these are here in order to prevent having to import the entire
 * <math.h> everywhere
 */

f32 msin(f32 x)
{
    return sinf(x);
}

f32 mcos(f32 x)
{
    return cosf(x);
}

f32 mtan(f32 x)
{
    return tanf(x);
}

f32 macos(f32 x)
{
    return acosf(x);
}

f32 msqrt(f32 x)
{
    return sqrtf(x);
}

f32 mabs(f32 x)
{
    return fabsf(x);
}

i32 mrandom()
{   
    if (!rand_seeded)
    {
        srand((u32)platform_get_absolute_time());
        rand_seeded = true;
    }
    return rand();
}

i32 mrandom_in_range(i32 min, i32 max)
{
    if (!rand_seeded)
    {
        srand((u32)platform_get_absolute_time());
        rand_seeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 fmrandom()
{
    return (float)mrandom() / (f32)RAND_MAX;
}

f32 fmrandom_in_range(f32 min, f32 max)
{
    return min + ((float)mrandom() / ((f32)RAND_MAX / (max - min)));
}