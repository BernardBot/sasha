#ifndef POSITION_H_
#define POSITION_H_

#include <stdint.h>
#include "definitions.h"

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
    uint64_t color[COLOR_N];
    uint64_t piece[PIECE_N];

    int pieceType[SQUARE_N];

    struct State *state;
};

void   doMove(uint16_t move, struct Position *pos, struct State *newState);
void undoMove(uint16_t move, struct Position *pos);

int parseInteger(char *s);
int parseSquare(char file, char rank);
int parseCastling(char *s);

void parseFen(char *fen, struct Position *pos);

#endif // POSITION_H_