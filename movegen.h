#ifndef MOVEGEN_H_
#define MOVEGEN_H_

#include <stdint.h>
#include "position.h"

uint16_t* makePromotions(int from, int to, uint16_t* moveList);

uint16_t* generatePawnMoves(struct Position *pos, uint16_t* moveList);
uint16_t* generatePieceMoves(struct Position *pos, uint16_t *moveList);
uint16_t* generateCastleMoves(struct Position *pos, uint16_t *moveList);

uint16_t* generatePseudoMoves(struct Position *pos, uint16_t *moveList);

#endif