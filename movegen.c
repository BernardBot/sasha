#include <stdint.h>
#include <stdio.h>
#include "util.h"
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

uint16_t* generatePawnMoves
(
    struct Position *pos, uint16_t *moveList, 
    const int us,
    const uint64_t friends, const uint64_t enemies, const uint64_t empties,
    const int quiet
)
{
    const uint64_t TRANK3 = us == WHITE ? RANK3 : RANK6;
    const uint64_t TRANK7 = us == WHITE ? RANK7 : RANK2;

    const int up          = us == WHITE ? -8    : 8;
    const int upLeft      = us == WHITE ? -9    : 7;
    const int upRight     = us == WHITE ? -7    : 9;

    const uint64_t pawnsOn7    = pos->piece[PAWN] & friends &  TRANK7;
    const uint64_t pawnsNotOn7 = pos->piece[PAWN] & friends & ~TRANK7;

    uint64_t b1, b2, b3, ep;
    int to;

    if (quiet)
    {
        b1 = shift_bb(pawnsNotOn7, up) & empties;
        b2 = shift_bb(b1 & TRANK3, up) & empties;
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
    } else
    {
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

        if (pawnsOn7)
        {
            b1 = shift_bb(pawnsOn7, upRight) & NILEA & enemies;
            b2 = shift_bb(pawnsOn7, upLeft ) & NILEH & enemies;
            b3 = shift_bb(pawnsOn7, up)              & empties;
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

        if (pos->state->enpassant != NO_SQ)
        {
            ep = sq_bb(pos->state->enpassant);
            b1 = shift_bb(pawnsNotOn7, upRight) & NILEA & ep;
            b2 = shift_bb(pawnsNotOn7, upLeft)  & NILEH & ep;
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
    }

    return moveList;
}

uint16_t* generatePieceMoves
(
    struct Position *pos, uint16_t *moveList, 
    const uint64_t friends, const uint64_t empties, const uint64_t targets
)
{
    uint64_t b, c;
    int from, to;

    b = pos->piece[KNIGHT] & friends;
    while (b)
    {
        from = __builtin_ctzll(b);
        c = knightLookup(from) & targets;
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
        c = kingLookup(from) & targets;
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
        c = rookLookup(from, empties) & targets;
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
        c = bishopLookup(from, empties) & targets;
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
uint16_t* generateCastleMoves
(
    struct Position *pos, uint16_t *moveList, 
    const int us, const int them, 
    const uint64_t empties
)
{
    // assume king is not in check
    if ( OOO[us]      & pos->state->castling              && // castling flag set?
        (OOO_MASK[us] & empties) == OOO_MASK[us]          && // squares between rook and king empty?
        (pos->pieceType[BRANK[us][A8]] % PIECE_N == ROOK) && // rook still on original square?
        !squareIsAttacked(BRANK[us][C8], them, pos)       && // squares for king not attacked?
        !squareIsAttacked(BRANK[us][D8], them, pos))
    {
        *moveList++ = (BRANK[us][E8]) | (BRANK[us][C8] << 6) | (CASTLING << 12);
    }
    if ( OO[us]       & pos->state->castling              &&
        (OO_MASK[us]  & empties) == OO_MASK[us]           &&
        (pos->pieceType[BRANK[us][H8]] % PIECE_N == ROOK) &&
        !squareIsAttacked(BRANK[us][F8], them, pos)       && 
        !squareIsAttacked(BRANK[us][G8], them, pos))
    {
        *moveList++ = (BRANK[us][E8]) | (BRANK[us][G8] << 6) | (CASTLING << 12);
    }

    return moveList;
}

uint16_t* generatePseudoMoves(struct Position *pos, uint16_t *moveList)
{
    const int us   = pos->state->turn;
    const int them = !us;

    const uint64_t friends    = pos->color[us];
    const uint64_t enemies    = pos->color[them];
    const uint64_t empties    = ~(friends | enemies);
    const uint64_t notFriends = ~friends;

    
    moveList = generatePawnMoves  (pos, moveList, us,       friends, enemies, empties, 0);
    moveList = generatePawnMoves  (pos, moveList, us,       friends, enemies, empties, 1);
    moveList = generatePieceMoves (pos, moveList,           friends,          empties, notFriends);
    moveList = generateCastleMoves(pos, moveList, us, them,                   empties);

    return moveList;
}

uint16_t* generateLegalMoves(struct Position *pos, uint16_t *moveList)
{
    const int us   = pos->state->turn;
    const int them = !us;

    const uint64_t friends    = pos->color[us];
    const uint64_t enemies    = pos->color[them];
    const uint64_t empties    = ~(friends | enemies);
    const uint64_t notFriends = ~friends;

    uint16_t *legalList = moveList;
    uint16_t *end       = moveList;
    struct State tempState;

    const uint64_t kingSquare = pos->piece[KING] & friends;
    const int kingSq          = __builtin_ctzll(kingSquare);

    // noisy moves
    end = generatePawnMoves (pos, end, us, friends, enemies, empties, 0);
    end = generatePieceMoves(pos, end,     friends, empties,          enemies);
    // quiet moves
    end = generatePawnMoves (pos, end, us, friends, enemies, empties, 1);
    end = generatePieceMoves(pos, end,     friends, empties,          empties);

    if (squareIsAttacked(kingSq, them, pos)) // we are in check
    {
        for (; moveList < end; moveList++)
        {
            doMove(*moveList, pos, &tempState);
            if (!squareIsAttacked(__builtin_ctzll(pos->piece[KING] & pos->color[us]), them, pos)) // make sure king is not attacked
            {
                *legalList++ = *moveList;
            }
            undoMove(*moveList, pos);
        }        
    } else
    {
        const uint64_t blockers = bishopLookup(kingSq, empties) | rookLookup(kingSq, empties);
        for (; moveList < end; moveList++)
        {
            if ((*moveList & 0b111111) == kingSq || sq_bb(*moveList & 0b111111) & blockers || ((*moveList >> 12) & 0b11) == ENPASSANT)
            {
                doMove(*moveList, pos, &tempState);
                if (!squareIsAttacked(__builtin_ctzll(pos->piece[KING] & pos->color[us]), them, pos))
                {
                    *legalList++ = *moveList;
                }
                undoMove(*moveList, pos);
            } else
            {
                *legalList++ = *moveList;
            }
        }
        legalList = generateCastleMoves(pos, legalList, us, them, empties);
    }

    return legalList;
}

uint16_t* generateNoisyMoves(struct Position *pos, uint16_t *moveList)
{
    const int us   = pos->state->turn;
    const int them = !us;

    const uint64_t friends    = pos->color[us];
    const uint64_t enemies    = pos->color[them];
    const uint64_t empties    = ~(friends | enemies);
    const uint64_t notFriends = ~friends;

    uint16_t *legalList = moveList;
    uint16_t *end       = moveList;
    struct State tempState;

    const uint64_t kingSquare = pos->piece[KING] & friends;
    const int kingSq          = __builtin_ctzll(kingSquare);

    // noisy moves
    end = generatePawnMoves (pos, end, us, friends, enemies, empties, 0);
    end = generatePieceMoves(pos, end,     friends, empties,          enemies);

    if (squareIsAttacked(kingSq, them, pos)) // we are in check
    {
        for (; moveList < end; moveList++)
        {
            doMove(*moveList, pos, &tempState);
            if (!squareIsAttacked(__builtin_ctzll(pos->piece[KING] & pos->color[us]), them, pos)) // make sure king is not attacked
            {
                *legalList++ = *moveList;
            }
            undoMove(*moveList, pos);
        }        
    } else
    {
        const uint64_t blockers = bishopLookup(kingSq, empties) | rookLookup(kingSq, empties);
        for (; moveList < end; moveList++)
        {
            if ((*moveList & 0b111111) == kingSq || sq_bb(*moveList & 0b111111) & blockers || ((*moveList >> 12) & 0b11) == ENPASSANT)
            {
                doMove(*moveList, pos, &tempState);
                if (!squareIsAttacked(__builtin_ctzll(pos->piece[KING] & pos->color[us]), them, pos))
                {
                    *legalList++ = *moveList;
                }
                undoMove(*moveList, pos);
            } else
            {
                *legalList++ = *moveList;
            }
        }
    }

    return legalList;
}

uint64_t perft(int depth, struct Position *pos)
{
    uint16_t moveList[MAX_MOVES];
    uint16_t *begin = moveList;
    uint16_t *end   = generateLegalMoves(pos, moveList);
    uint64_t nodes;
    struct State newState;

    if (depth <  1) return 0; // don't shoot yourself in the foot
    if (depth == 1) return end - begin;

    for (nodes = 0; begin < end; begin++)
    {
        doMove(*begin, pos, &newState);
        nodes += perft(depth - 1, pos);
        undoMove(*begin, pos);
    }

    return nodes;
}