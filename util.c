#include "util.h"
#include "definitions.h"
#include "position.h"
#include <stdio.h>
#include <stdint.h>

void printU64(uint64_t board)
{
    int i;
    for (i = 0; i < SQUARE_N; i++)
    {
        putchar((board >> i) & 1 ? 'X' : '.');
        if (i % 8 == 7)
        {
            printf("\n");
        }
    }
    printf("\n");
}

void printMove(uint16_t move)
{
    int from =  move        & 0b111111;
    int to   = (move >> 6)  & 0b111111;
    int tag  = (move >> 12) & 0b11;
    int prom = (move >> 14) & 0b11;
    printf("%s%s", squareString[from], squareString[to]);
}

void printMoveList(uint16_t *moveList, const uint16_t *end)
{
    int t;
    for (t = *moveList & 0b111111; moveList < end; moveList++)
    {
        if (t != (*moveList & 0b111111))
        {
            printf("\n");
            t = *moveList & 0b111111;
        }
        printMove(*moveList);
    }
    printf("\n");
}

void printBoard(int pieceType[])
{
    int i;
    printf("\n");
    for (i = 0; i < SQUARE_N; i++)
    {
        if (i % 8 == 0)
        {
            printf("%c  ", '8' - i / 8);
        }

        printf("%c  ", pieceTypeChar[pieceType[i]]);

        if (i % 8 == 7) 
        {
            printf("\n");
        }
    }
    printf("   a  b  c  d  e  f  g  h\n\n");
}

void printState(struct State state)
{
    printf("turn             \t%d\n",   state.turn);
    printf("enpassant        \t%d\n",   state.enpassant);
    printf("castling         \t%d\n",   state.castling);
    printf("movecount        \t%d\n",   state.movecount);
    printf("halfmovecount    \t%d\n",   state.halfmovecount);
    printf("capturedPieceType\t%d\n",   state.capturedPieceType);
    printf("capturedSquare   \t%d\n",   state.capturedSquare);
    printf("zobrist          \t%llu\n", state.zobrist);
}