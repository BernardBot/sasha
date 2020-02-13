#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

void printU64(uint64_t board);
void printBoard(int pieceTypes[]);
void printMove(uint16_t move);
void printMoveList(uint16_t *moveList);

#endif // UTIL_H_