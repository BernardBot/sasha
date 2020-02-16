#include "util.h"
#include "perft.h"
#include "lookup.h"
#include "position.h"
#include "movegen.h"
#include <stdio.h>

uint64_t perft(int depth, struct Position *pos)
{
    uint16_t moveList[256];
    uint16_t *begin = moveList;
    uint16_t *end;
    uint64_t nodes;
    struct State newState;

    end = generateLegalMoves(pos, moveList);
    
    if (depth == 1) return end - begin;

    for (nodes = 0; begin < end; begin++)
    {
        doMove(*begin, pos, &newState);
        nodes += perft(depth - 1, pos);
        undoMove(*begin, pos);
    }

    return nodes;
}