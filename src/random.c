#include <stdint.h>
#include <stdlib.h>

#include <dht/random.h>
#include <lcthw/dbg.h>

#define RANDOM_STATE_LEN 256

RandomState *
RandomState_Create(unsigned int seed)
{
    RandomState *state = NULL;

    state = calloc(1, sizeof(RandomState));
    check_mem(state);

    state->seed = seed;

    return state;

error:
    free(state);

    return NULL;
}

void 
RandomState_Destroy(RandomState *randomState)
{
    if (randomState == NULL)
        return;

    free(randomState);
}

int 
Random_Fill(RandomState *randomState, char *buf, size_t len)
{
    size_t i = 0;

    for (i = 0; i + sizeof(int32_t) <= len; i += sizeof(int32_t))
    {
        buf[i] = rand_r(&randomState->seed);
    }

    if (i == len) return 0;

    int r = rand_r(&randomState->seed);

    while (i < len)
    {
        buf[i++] = r & 0xff;
        r = r >> 8;
    }

    return 0;
}
