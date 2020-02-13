#ifndef ATTACKS_H
#define ATTACKS_H

#include <stdint.h>

uint64_t wPawnAttacks(uint64_t pawns);
uint64_t bPawnAttacks(uint64_t pawns);
uint64_t knightAttacks(uint64_t knights);
uint64_t kingAttacks(uint64_t king);
uint64_t slidingAttacks(uint64_t sliders, uint64_t empty, int dir8);
uint64_t bishopAttacks(uint64_t bishops, uint64_t empty);
uint64_t rookAttacks(uint64_t rooks, uint64_t empty);

#endif // ATTACKS_H_