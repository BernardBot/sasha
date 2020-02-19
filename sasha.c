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
#include "search.h"

int main()
{
    initLookup();
    initUciInfo();

    struct Position pos = {0};
    struct State firstState = {0};
    struct State stateList[256];
    pos.state = &firstState;

    char *fen = "8/8/8/8/8/2K5/7R/k7 w - - 0 1";
    parseFen(fen, &pos);
    // 1 move is 2 halfmoves
    // mate in 5 is halfmoves 10/11 moves

    // printBoard(pos.pieceType);
    // int eval = search(&pos, 10, -INF, INF);
    // printf("%d %d %d\n", eval, eval - INF, INF);

    // struct Info info;
    // printMove(bestMove(&pos, info));

    // struct State newState;
    // doMove((A2) | (A3 << 6), &pos, &newState);
    // printState(*pos.state);

    // char *fen2 = "rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 2";
    // parseFen(fen2, &pos);
    // printState(*pos.state);

    uciLoop(&pos, stateList);

    return 0;
}
