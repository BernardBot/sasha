#include <stdint.h>
#include "position.h"
#include "lookup.h"
#include "definitions.h"
#include "movegen.h"

uint16_t* makePromotions(int from, int to, uint16_t* moveList)
{
    *moveList++ = (from) | (to << 6) | (PROMOTION << 12) | (QUEEN  << 14);
    *moveList++ = (from) | (to << 6) | (PROMOTION << 12) | (ROOK   << 14);
    *moveList++ = (from) | (to << 6) | (PROMOTION << 12) | (BISHOP << 14);
    *moveList++ = (from) | (to << 6) | (PROMOTION << 12) | (KNIGHT << 14);

    return moveList;
}

uint16_t* generatePawnMoves(struct Position *pos, uint16_t *moveList)
{
    const int us = pos->state->turn;
    const int them = !us;

    const uint64_t friends = pos->color[us];
    const uint64_t enemies = pos->color[them];
    const uint64_t empties = ~(friends | enemies);

    const uint64_t TRANK3 = us == WHITE ? RANK3 : RANK6;
    const uint64_t TRANK7 = us == WHITE ? RANK7 : RANK2;
    const uint64_t TRANK5 = us == WHITE ? RANK5 : RANK4;

    const int up          = us == WHITE ? -8    : 8;
    const int upLeft      = us == WHITE ? -9    : 7;
    const int upRight     = us == WHITE ? -7    : 9;

    const uint64_t pawnsOn7    = pos->piece[PAWN] & friends &  TRANK7;
    const uint64_t pawnsNotOn7 = pos->piece[PAWN] & friends & ~TRANK7;

    uint64_t b1 = shift_bb(pawnsNotOn7, up) & empties;
    uint64_t b2 = shift_bb(b1 & TRANK3, up) & empties;

    while (b1)
    {
        int to = __builtin_ctzll(b1);
        *moveList++ = (to - up) | (to << 6);
        b1 &= b1 - 1;
    }
    while (b2)
    {
        int to = __builtin_ctzll(b2);
        *moveList++ = (to - up - up) | (to << 6);
        b2 &= b2 - 1;
    }

    if (pawnsOn7)
    {
        uint64_t b1 = shift_bb(pawnsOn7, upRight) & NILEA & enemies;
        uint64_t b2 = shift_bb(pawnsOn7, upLeft ) & NILEH & enemies;
        uint64_t b3 = shift_bb(pawnsOn7, up)              & empties;

        while (b1)
        {
            int to = __builtin_ctzll(b1);
            moveList = makePromotions(to - upRight, to, moveList);
            b1 &= b1 - 1;
        }
        while (b2)
        {
            int to = __builtin_ctzll(b2);
            moveList = makePromotions(to - upLeft, to, moveList);
            b2 &= b2 - 1;
        }
        while (b3)
        {
            int to = __builtin_ctzll(b3);
            moveList = makePromotions(to - up, to, moveList);
            b3 &= b3 - 1;
        }
    }

    b1 = shift_bb(pawnsNotOn7, upRight) & NILEA & enemies;
    b2 = shift_bb(pawnsNotOn7, upLeft ) & NILEH & enemies;
    
    while (b1)
    {
        int to = __builtin_ctzll(b1);
        *moveList++ = (to - upRight) | (to << 6);
        b1 &= b1 - 1;
    }
    while (b2)
    {
        int to = __builtin_ctzll(b2);
        *moveList++ = (to - upLeft) | (to << 6);
        b2 &= b2 - 1;
    }

    if (pos->state->enpassant != -1)
    {
        uint64_t ep = file_bb(pos->state->enpassant) & TRANK5;
        uint64_t b1 = shift_bb(pawnsNotOn7, upRight);
        uint64_t b2 = shift_bb(pawnsNotOn7, upLeft);

        while (b1)
        {
            int to = __builtin_ctzll(b1);
            *moveList++ = (to - upRight) | (to << 6) | (ENPASSANT << 12);
            b1 &= b1 - 1;
        }
        while (b2)
        {
            int to = __builtin_ctzll(b2);
            *moveList++ = (to - upLeft) | (to << 6) | (ENPASSANT << 12);
            b2 &= b2 - 1;
        }
    }

    return moveList;
}

uint16_t* generatePieceMoves(struct Position *pos, uint16_t *moveList)
{
    const int us = pos->state->turn;
    const int them = !us;

    const uint64_t friends = pos->color[us];
    const uint64_t enemies = pos->color[them];
    const uint64_t empties = ~(friends | enemies);
 
    uint64_t b, c;
    int pc, from, to;

    for (pc = 0; pc < PAWN; pc++)
    {
        b = pos->piece[pc] & friends;
        while (b)
        {
            from = __builtin_ctzll(b);

            switch (pc)
            {
            case KNIGHT: c = knightLookup(from)          & ~friends; break;
            case KING:   c = kingLookup(from)            & ~friends; break;
            case ROOK:   c = rookLookup(from, empties)   & ~friends; break;
            case BISHOP: c = bishopLookup(from, empties) & ~friends; break;
            case QUEEN:  c = (rookLookup(from, empties) | bishopLookup(from, empties)) & ~friends; break;
            default:     c = 0; break;
            }

            while (c)
            {
                to = __builtin_ctzll(c);
                *moveList++ = (from) | (to << 6);
                c &= c - 1;
            }
            
            b &= b - 1;
        }
    }

    return moveList;
}
uint16_t* generateCastleMoves(struct Position *pos, uint16_t *moveList)
{
    const uint64_t us = pos->state->turn;
    const uint64_t them = !us;
    const uint64_t empties = ~(pos->color[us] | pos->color[them]);

    const uint64_t OO_MASK  = us == WHITE ? WOO_MASK  : BOO_MASK;
    const uint64_t OOO_MASK = us == WHITE ? WOOO_MASK : BOOO_MASK;
    
    const int OO            = us == WHITE ? WOO       : BOO;
    const int OOO           = us == WHITE ? WOOO      : BOOO;

    const int A             = us == WHITE ? A1        : A8;
    const int B             = us == WHITE ? B1        : B8;
    const int C             = us == WHITE ? C1        : C8;
    const int D             = us == WHITE ? D1        : D8;
    // const int E             = us == WHITE ? E1        : E8;
    const int F             = us == WHITE ? F1        : F8;
    const int G             = us == WHITE ? G1        : G8;
    const int H             = us == WHITE ? H1        : H8;

    // assume king is not in check
    if ( OO       & pos->state->castling &&
        (OO_MASK  & empties) == OO_MASK  &&
        !squareIsAttacked(F, them, pos)  &&
        !squareIsAttacked(G, them, pos))
    {
        *moveList++ = (H) | (F << 6) | (CASTLING << 12);
    }
    if ( OOO      & pos->state->castling &&
        (OOO_MASK & empties) == OOO_MASK &&
        !squareIsAttacked(C, them, pos)  &&
        !squareIsAttacked(D, them, pos))
    {
        *moveList++ = (A) | (D << 6) | (CASTLING << 12);
    }
    
    return moveList;
}

uint16_t* generatePseudoMoves(struct Position *pos, uint16_t *moveList)
{
    moveList = generatePawnMoves(pos, moveList);
    moveList = generatePieceMoves(pos, moveList);
    moveList = generateCastleMoves(pos, moveList);

    return moveList;
}

