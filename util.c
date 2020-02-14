#include "util.h"
#include "definitions.h"
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
    int from = move & 0b111111;
    int to = (move >> 6) & 0b111111;
    printf(" %s%s", squareString[from], squareString[to]);
}

void printMoveList(uint16_t *moveList)
{
    int t;
    for (t = *moveList & 0b111111; *moveList; moveList++)
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
    putchar('\n');
    int i;
    for (i = 0; i < SQUARE_N; i++)
    {
        if (i % 8 == 0)
        {
            putchar('8' - i / 8);
        }
        putchar(pieceTypeChar[pieceType[i]]);
        if (i % 8 == 7) 
        {
            putchar('\n');
        }
    }
    printf(" abcdefgh\n\n");
}
