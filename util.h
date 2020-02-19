#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include "position.h"

void printU64(uint64_t board);
void printBoard(int pieceType[]);
void printMove(uint16_t move);
void printMoveList(uint16_t *begin, const uint16_t *end);
void printState(struct State state);

#endif // UTIL_H_