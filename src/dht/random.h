#ifndef _dht_random_h
#define _dht_random_h

#include <stdint.h>
#include <stdlib.h>

typedef struct RandomState {
    unsigned int seed;
} RandomState;

RandomState *RandomState_Create(unsigned int seed);
void RandomState_Destroy(RandomState *randomState);

int Random_Fill(RandomState *randomState, char *buf, size_t len);

#endif

