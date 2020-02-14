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
    const int us   = pos->state->turn;
    const int them = !us;

    const uint64_t friends = pos->color[us];
    const uint64_t enemies = pos->color[them];
    const uint64_t empties = ~(friends | enemies);

    const uint64_t TRANK3 = us == WHITE ? RANK3 : RANK6;
    const uint64_t TRANK7 = us == WHITE ? RANK7 : RANK2;

    const int up          = us == WHITE ? -8    : 8;
    const int upLeft      = us == WHITE ? -9    : 7;
    const int upRight     = us == WHITE ? -7    : 9;

    const uint64_t pawnsOn7    = pos->piece[PAWN] & friends &  TRANK7;
    const uint64_t pawnsNotOn7 = pos->piece[PAWN] & friends & ~TRANK7;

    uint64_t b1 = shift_bb(pawnsNotOn7, up) & empties;
    uint64_t b2 = shift_bb(b1 & TRANK3, up) & empties;

    int to;
    while (b1)
    {
        to = __builtin_ctzll(b1);
        *moveList++ = (to - up) | (to << 6);
        b1 &= b1 - 1;
    }
    while (b2)
    {
        to = __builtin_ctzll(b2);
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
            to = __builtin_ctzll(b1);
            moveList = makePromotions(to - upRight, to, moveList);
            b1 &= b1 - 1;
        }
        while (b2)
        {
            to = __builtin_ctzll(b2);
            moveList = makePromotions(to - upLeft, to, moveList);
            b2 &= b2 - 1;
        }
        while (b3)
        {
            to = __builtin_ctzll(b3);
            moveList = makePromotions(to - up, to, moveList);
            b3 &= b3 - 1;
        }
    }

    b1 = shift_bb(pawnsNotOn7, upRight) & NILEA & enemies;
    b2 = shift_bb(pawnsNotOn7, upLeft ) & NILEH & enemies;
    
    while (b1)
    {
        to = __builtin_ctzll(b1);
        *moveList++ = (to - upRight) | (to << 6);
        b1 &= b1 - 1;
    }
    while (b2)
    {
        to = __builtin_ctzll(b2);
        *moveList++ = (to - upLeft) | (to << 6);
        b2 &= b2 - 1;
    }

    if (pos->state->enpassant != -1)
    {
        uint64_t ep = sq_bb(pos->state->enpassant);
        uint64_t b1 = shift_bb(pawnsNotOn7, upRight) & ep;
        uint64_t b2 = shift_bb(pawnsNotOn7, upLeft)  & ep;

        while (b1)
        {
            to = __builtin_ctzll(b1);
            *moveList++ = (to - upRight) | (to << 6) | (ENPASSANT << 12);
            b1 &= b1 - 1;
        }
        while (b2)
        {
            to = __builtin_ctzll(b2);
            *moveList++ = (to - upLeft) | (to << 6) | (ENPASSANT << 12);
            b2 &= b2 - 1;
        }
    }

    return moveList;
}

uint16_t* generatePieceMoves(struct Position *pos, uint16_t *moveList)
{
    const int us   = pos->state->turn;
    const int them = !us;

    const uint64_t friends    = pos->color[us];
    const uint64_t enemies    = pos->color[them];
    const uint64_t empties    = ~(friends | enemies);
    const uint64_t notFriends = ~friends;
 
    uint64_t b, c;
    int from, to;

    b = pos->piece[KNIGHT] & friends;
    while (b)
    {
        from = __builtin_ctzll(b);
        c = knightLookup(from) & notFriends;
        while (c)
        {
            to = __builtin_ctzll(c);
            *moveList++ = (from) | (to << 6);
            c &= c - 1;
        }
        b &= b - 1;
    }

    b = pos->piece[KING] & friends;
    while (b)
    {
        from = __builtin_ctzll(b);
        c = kingLookup(from) & notFriends;
        while (c)
        {
            to = __builtin_ctzll(c);
            *moveList++ = (from) | (to << 6);
            c &= c - 1;
        }
        b &= b - 1;
    }

    b = (pos->piece[ROOK] | pos->piece[QUEEN]) & friends;
    while (b)
    {
        from = __builtin_ctzll(b);
        c = rookLookup(from, empties) & notFriends;
        while (c)
        {
            to = __builtin_ctzll(c);
            *moveList++ = (from) | (to << 6);
            c &= c - 1;
        }
        b &= b - 1;
    }

    b = (pos->piece[BISHOP] | pos->piece[QUEEN]) & friends;
    while (b)
    {
        from = __builtin_ctzll(b);
        c = bishopLookup(from, empties) & notFriends;
        while (c)
        {
            to = __builtin_ctzll(c);
            *moveList++ = (from) | (to << 6);
            c &= c - 1;
        }
        b &= b - 1;
    }

    return moveList;
}
uint16_t* generateCastleMoves(struct Position *pos, uint16_t *moveList)
{
    const uint64_t us      = pos->state->turn;
    const uint64_t empties = ~(pos->color[us] | pos->color[!us]);

    // assume king is not in check
    if ( OOO[us]      & pos->state->castling              &&
        (OOO_MASK[us] & empties) == OOO_MASK[us]          &&
        (pos->pieceType[BRANK[us][A8]] % PIECE_N == ROOK) &&
        !squareIsAttacked(BRANK[us][C8], !us, pos)        &&
        !squareIsAttacked(BRANK[us][D8], !us, pos))
    {
        *moveList++ = (BRANK[us][E8]) | (BRANK[us][C8] << 6) | (CASTLING << 12);
    }
    if ( OO[us]       & pos->state->castling              &&
        (OO_MASK[us]  & empties) == OO_MASK[us]           &&
        (pos->pieceType[BRANK[us][H8]] % PIECE_N == ROOK) &&
        !squareIsAttacked(BRANK[us][F8], !us, pos)        &&
        !squareIsAttacked(BRANK[us][G8], !us, pos))
    {
        *moveList++ = (BRANK[us][E8]) | (BRANK[us][G8] << 6) | (CASTLING << 12);
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

