#ifndef MOVEGEN_H_
#define MOVEGEN_H_

#include <stdint.h>
#include "position.h"

uint16_t* makePromotions(int from, int to, uint16_t* moveList);

uint16_t* generatePawnMoves
(
    struct Position *pos, uint16_t *moveList, 
    const int us,
    const uint64_t friends, const uint64_t enemies, const uint64_t empties,
    const int quiet
);
uint16_t* generatePieceMoves
(
    struct Position *pos, uint16_t *moveList, 
    const uint64_t friends, const uint64_t empties, const uint64_t notFriends
);
uint16_t* generateCastleMoves
(
    struct Position *pos, uint16_t *moveList, 
    const int us, const int them, 
    const uint64_t empties
);

uint16_t* generatePseudoMoves(struct Position *pos, uint16_t *moveList);
uint16_t* generateLegalMoves(struct Position *pos, uint16_t *moveList);
uint16_t* generateNoisyMoves(struct Position *pos, uint16_t *moveList);
uint16_t* generateQuietMoves(struct Position *pos, uint16_t *moveList);

uint64_t perft(int depth, struct Position *pos);

#endif