#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "attacks.h"
#include "definitions.h"
#include "lookup.h"
#include "position.h"
#include "movegen.h"
#include "position.h"
#include "uci.h"
#include "hashtable.h"

int main()
{
    initLookup();

    char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    struct Position pos = {0};
    struct State firstState = {0};

    pos.state = &firstState;

    parseFen(fen, &pos);
    printBoard(pos.pieceType);
    printState(*pos.state);
    for (int i = 1; i < 7; i++)
    {
        printf("%llu\n", perft(i, &pos));
    }

    return 0;
}
