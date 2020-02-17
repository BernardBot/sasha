#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "lookup.h"
#include "definitions.h"
#include "attacks.h"
#include "position.h"
#include "util.h"

uint64_t bishopLookup(int sq, uint64_t empty)
{
    empty  &= bishopMasks[sq];
    empty  *= bishopMagics[sq];
    empty >>= 55; // 64 - 9
    return BISHOPATTACKS[sq][empty];
}
uint64_t rookLookup(int sq, uint64_t empty)
{
    empty  &= rookMasks[sq];
    empty  *= rookMagics[sq];
    empty >>= 52; // 64 - 12;
    return ROOKATTACKS[sq][empty];
}
uint16_t squareIsAttacked(int sq, int them, struct Position *pos)
{
    const uint64_t enemies =   pos->color[ them];
    const uint64_t empties = ~(pos->color[!them] | enemies);

    const uint64_t pawns   =  pos->piece[PAWN]                        & enemies;
    const uint64_t knights =  pos->piece[KNIGHT]                      & enemies;
    const uint64_t king    =  pos->piece[KING]                        & enemies;
    const uint64_t bishops = (pos->piece[BISHOP] | pos->piece[QUEEN]) & enemies;
    const uint64_t rooks   = (pos->piece[ROOK]   | pos->piece[QUEEN]) & enemies;

    return (pawnLookup(sq, !them)     & pawns)   ||
           (knightLookup(sq)          & knights) ||
           (kingLookup(sq)            & king)    ||
           (bishopLookup(sq, empties) & bishops) ||
           (rookLookup(sq, empties)   & rooks);
}

uint64_t rand64()
{
    uint64_t r1, r2, r3, r4;

    r1 = (uint64_t)random() & 0xffff;
    r2 = (uint64_t)random() & 0xffff;
    r3 = (uint64_t)random() & 0xffff;
    r4 = (uint64_t)random() & 0xffff;

    return r1 | (r2 << 16) | (r3 << 32) | (r3 << 48);
}
uint64_t rand64Sparse()
{
    return rand64() & rand64() & rand64();
}
int popcount(uint64_t x)
{
    int i;
    for (i = 0; x; i++)
    {
        x &= x - 1;
    }
    return i;
}

void initMagic(int sq, uint64_t *mask, uint64_t *magic, uint64_t attackTable[], int isRook)
{
    int i, j, size, shift;
    uint64_t empties[4096], attacks[4096], b;

    uint64_t square =   sq_bb(sq);
    uint64_t file   = file_bb(sq);
    uint64_t rank   = rank_bb(sq);

    uint64_t fileEdges = (FILEA | FILEH) & ~file;
    uint64_t rankEdges = (RANK1 | RANK8) & ~rank;
    uint64_t edges     = (fileEdges | rankEdges);

    *mask = isRook ? rookAttacks  (square, ~0) & ~edges :
                     bishopAttacks(square, ~0) & ~edges;
    shift = isRook ? 52 : 55;

    b = size = 0;
    do
    {
        empties[size] = b;
        attacks[size] = isRook ? rookAttacks  (square, b) :
                                 bishopAttacks(square, b);
        size++;
        b = (b - *mask) & *mask;
    } while (b);

    for(;;)
    {
        *magic = rand64Sparse();

        if (popcount((*mask * *magic) >> 56) < 6)
        {
            continue;
        }

        for (j = 0; j < (1 << (64 - shift)); j++)
        {
            attackTable[j] = 0;
        }

        for (i = 0; ; i++)
        {
            if (i == size)
            {
                return;
            }

            j = (empties[i] * *magic) >> shift;
            
            if (attackTable[j] == 0)
            {
                attackTable[j] = attacks[i];
            }
            else if (attackTable[j] != attacks[i])
            {
                break;
            }
        }
    }
}

void initLookup()
{
    srand(0x5a54a); // set seed

    int sq, pct, cstl;
    for (sq = 0; sq < SQUARE_N; sq++)
    {
        PAWNATTACKS[WHITE][sq] = wPawnAttacks(sq_bb(sq));
        PAWNATTACKS[BLACK][sq] = bPawnAttacks(sq_bb(sq));
        KINGATTACKS[sq]        = kingAttacks(sq_bb(sq));
        KNIGHTATTACKS[sq]      = knightAttacks(sq_bb(sq));

        initMagic(sq, &rookMasks[sq],   &rookMagics[sq],   ROOKATTACKS[sq],   1);
        initMagic(sq, &bishopMasks[sq], &bishopMagics[sq], BISHOPATTACKS[sq], 0);

        ZOBRISTENPASSANT[sq] = rand64();

        for (pct = 0; pct < PIECETYPE_N; pct++)
        {
            ZOBRISTPIECES[pct][sq] = rand64();
        }
    }

    for (cstl = 0; cstl < CASTLE_N; cstl++)
    {
        ZOBRISTCASTLES[cstl] = rand64();
    }
}

uint64_t zobristKey(struct Position *pos)
{
    uint64_t key = 0;
    int sq;

    for (sq = 0; sq < SQUARE_N; sq++)
    {
        key ^= ZOBRISTPIECES[pos->pieceType[sq]][sq];
    }

    if (pos->state->enpassant != -1)
    {
        key ^= ZOBRISTENPASSANT[pos->state->enpassant];
    } 

    key ^= ZOBRISTCASTLES[pos->state->castling];

    return key;
}
