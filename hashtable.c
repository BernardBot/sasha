#include "hashtable.h"
#include <stdint.h>

struct Entry TTABLE[TABLESIZE] = {0}; // initialize global transposition table

int getTT(uint64_t key, uint16_t *move, int *eval)
{
    uint16_t index = key >> TABLESHIFT;
    if (TTABLE[index].key == key)
    {
        *move = TTABLE[index].move;
        *eval = TTABLE[index].eval;
        return 1; // success
    }
    return 0; // failure
}

void putTT(uint64_t key, uint16_t move, int eval)
{
    uint16_t index = key >> TABLESHIFT;
    TTABLE[index].key  = key;
    TTABLE[index].move = move;
    TTABLE[index].eval = eval;
}
