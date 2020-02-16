#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

void printU64(uint64_t board);
void printBoard(int pieceType[]);
void printMove(uint16_t move);
void printMoveList(uint16_t *moveList, const uint16_t *end);

#endif // UTIL_H_