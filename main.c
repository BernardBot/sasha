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
    // TODO: take a break :)
    struct Position pos;
    struct State state;
    struct State newState[100];
    pos.state = &state;
    char *fen = "r1bq1rk1/2p1bppp/p1n2n2/1p1pp3/4P3/1BP2N2/PP1P1PPP/RNBQR1K1 w - - 0 9";
    uint16_t moveList[256] = {0};
    uint16_t moves[100];
    uint16_t *mPtr = moves;

    parseFen(fen, &pos);
    initLookup();

    char s[100];
    int i = 0;
    for(;;)
    {
        for (int j = 0; j < 256; j++)
        {
            moveList[j] = 0;
        }
        generatePseudoMoves(&pos, moveList);
        printBoard(pos.pieceType);
        printMoveList(moveList);

        printf(">");
        scanf("%s", s);
        if (s[0] == 'u')
        {
            if (mPtr == moves)
            {
                printf("NOTHING TO UNDO\n");
                continue;
            }
            undoMove(*--mPtr, &pos);
        } else
        {
            int fromSq = parseSquare(s[0], s[1]);
            int toSq   = parseSquare(s[2], s[3]);
            if (!(0 <= fromSq && fromSq < SQUARE_N) ||
                !(0 <= toSq   && toSq   < SQUARE_N))
                {
                    printf("ENTER LEGAL MOVE BITCH\n");
                    continue;
                }            
            int legal = 0;
            *mPtr = (fromSq) | (toSq << 6);
            for (int j = 0; j < 256; j++)
            {
                if (*mPtr == (moveList[j] & 0xfff))
                {
                    *mPtr = moveList[j];
                    legal = 1;
                    break;
                }
            }

            if (!legal)
            {
                printf("ENTER LEGAL MOVE BITCH\n");
                continue;
            }

            doMove(*mPtr++, &pos, &newState[i++]);
        }
    }

    return 0;
}
