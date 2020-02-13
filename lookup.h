#ifndef LOOKUP_H_
#define LOOKUP_H_

#include <stdint.h>
#include "position.h"
#include "definitions.h"

uint64_t PAWNATTACKS[COLOR_N][SQUARE_N];
uint64_t KNIGHTATTACKS[SQUARE_N];
uint64_t KINGATTACKS[SQUARE_N];

uint64_t bishopMagics[SQUARE_N];
uint64_t bishopMasks[SQUARE_N];
uint64_t BISHOPATTACKS[SQUARE_N][512]; // 512 = 2 ^ 9

uint64_t rookMagics[SQUARE_N];
uint64_t rookMasks[SQUARE_N];
uint64_t ROOKATTACKS[SQUARE_N][4096]; // 4096 = 2 ^ 12

uint64_t pawnLookup(int sq, int color);
uint64_t knightLookup(int sq);
uint64_t kingLookup(int sq);
uint64_t bishopLookup(int sq, uint64_t empty);
uint64_t rookLookup(int sq, uint64_t empty);

uint16_t squareIsAttacked(int sq, int color, struct Position *pos);

uint64_t rand64();
uint64_t rand64Sparse();
int popcount(uint64_t x);
void initMagic(int sq, uint64_t *mask, uint64_t *magic, uint64_t attackTable[], int isRook);
void initLookup();

#endif