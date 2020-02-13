#include "definitions.h"
#include "attacks.h"
#include <stdint.h>

uint64_t wPawnAttacks(uint64_t pawns)
{
    return ((pawns >> 7) & NILEA) | ((pawns >> 9) & NILEH);
}
uint64_t bPawnAttacks(uint64_t pawns)
{
    return ((pawns << 7) & NILEH) | ((pawns << 9) & NILEA);
}
uint64_t knightAttacks(uint64_t knights)
{
    uint64_t w1 = (knights >> 1) & NILEH;
    uint64_t w2 = (knights >> 2) & NILEGH;
    uint64_t e1 = (knights << 1) & NILEA;
    uint64_t e2 = (knights << 2) & NILEAB;
    uint64_t h1 = w1 | e1;
    uint64_t h2 = w2 | e2;
    return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
}
uint64_t kingAttacks(uint64_t king)
{
    uint64_t att = ((king << 1) & NILEA) | ((king >> 1) & NILEH);
    king |= att;
    return att | (king >> 8) | (king << 8);
}
uint64_t slidingAttacks(uint64_t sliders, uint64_t empty, int dir8)
{
    uint64_t flood = sliders;
    int s = SHIFT[dir8];
    empty &= WRAP[dir8];
    flood |= sliders = shift_bb(sliders, s) & empty;
    flood |= sliders = shift_bb(sliders, s) & empty;
    flood |= sliders = shift_bb(sliders, s) & empty;
    flood |= sliders = shift_bb(sliders, s) & empty;
    flood |= sliders = shift_bb(sliders, s) & empty;
    flood |=           shift_bb(sliders, s) & empty;
    return shift_bb(flood, s) & WRAP[dir8];
}
uint64_t bishopAttacks(uint64_t bishops, uint64_t empty)
{
    return slidingAttacks(bishops, empty, 0) |
           slidingAttacks(bishops, empty, 2) |
           slidingAttacks(bishops, empty, 4) |
           slidingAttacks(bishops, empty, 6);
}
uint64_t rookAttacks(uint64_t rooks, uint64_t empty)
{
    return slidingAttacks(rooks, empty, 1) |
           slidingAttacks(rooks, empty, 3) |
           slidingAttacks(rooks, empty, 5) |
           slidingAttacks(rooks, empty, 7);
}
