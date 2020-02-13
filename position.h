#ifndef POSITION_H_
#define POSITION_H_

#include <stdint.h>

struct State
{
    int turn;

    int enpassant;
    int castling;

    int movecount;
    int halfmovecount;

    int capturedPieceType;
    int capturedSquare;

    struct State *previousState;
};
struct Position
{
    uint64_t color[2];
    uint64_t piece[6];

    int pieceType[64];

    struct State *state;
};

int pieceOf(int pieceType);
int colorOf(int pieceType);

void doMove(uint16_t move, struct Position *pos, struct State *newState);
void undoMove(uint16_t move, struct Position *pos);

void readFen(char *fen, struct Position *pos);

#endif // POSITION_H_