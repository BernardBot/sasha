#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "attacks.h"
#include "definitions.h"
#include "lookup.h"
#include "position.h"
#include "movegen.h"
#include "position.h"

int main()
{
    struct Position pos;
    struct State state;
    struct State newState;
    pos.state = &state;
    char *fen = "8/8/8/8/3p4/8/4P3/8 w - - 0 1";
    uint16_t moveList[256] = {0};
    uint16_t *p;

    readFen(fen, &pos);
    initLookup();

    p = generatePseudoMoves(&pos, moveList);
    printBoard(pos.pieceType);
    printMoveList(moveList);

    uint16_t move = (B4) | (A3 << 6) | (ENPASSANT << 12);
    printf("%ld\n", p - moveList);
    doMove(moveList[1], &pos, &newState);
    
    for (int i = 0; i < 256; i++)
    {
        moveList[i] = 0;
    }

    generatePseudoMoves(&pos, moveList);
    printBoard(pos.pieceType);
    printMoveList(moveList);
    
    // printf("%d\n", pos.state->movecount);
    // printf("%d\n", pos.state->halfmovecount);
    // for (int p = 0; p < PIECE_N; p++)
    // {
    //     printf("%d\n", p);
    //     printU64(pos.piece[p]);
    // }
    // for (int c = 0; c < COLOR_N; c++)
    //     printU64(pos.color[c]);
    return 0;
}
