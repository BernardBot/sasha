#include "hashtable.h"
#include "definitions.h"
#include <stdint.h>

struct Entry TTABLE[TABLESIZE] = {0}; // initialize global transposition table

int getTT(uint64_t key, uint16_t *move, int *eval, int *depth, int *flag)
{
    uint16_t index = key >> TABLESHIFT;
    if (TTABLE[index].key == key)
    {
        *move  = TTABLE[index].move;
        *eval  = TTABLE[index].eval;
        *depth = TTABLE[index].depth;
        *flag  = TTABLE[index].flag;
        return 1; // success
    }
    return 0; // failure
}

void putTT(uint64_t key, uint16_t move, int eval, int depth, int flag)
{
    uint16_t index = key >> TABLESHIFT;
    TTABLE[index].key   = key;
    TTABLE[index].move  = move;
    TTABLE[index].eval  = eval;
    TTABLE[index].depth = depth;
    TTABLE[index].flag  = flag;
}

int evalFromTT(int eval, int height)
{
    return eval >=  MATE_IN_MAX ? eval - height
         : eval <= MATED_IN_MAX ? eval + height
         : eval;
}

int evalToTT(int eval, int height)
{
    return eval >=  MATE_IN_MAX ? eval + height
         : eval <= MATED_IN_MAX ? eval - height
         : eval;
}
