#include <stdio.h>
#include "util.h"
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "lookup.h"
#include "hashtable.h"
#include <stdlib.h>
#include "uci.h"

int evalPos(struct Position *pos)
{
    return (1 - 2 * pos->state->turn) *
    (
           3 * ((popcount(pos->piece[KNIGHT] & pos->color[WHITE])) - (popcount(pos->piece[KNIGHT] & pos->color[BLACK]))) +
           5 * ((popcount(pos->piece[ROOK]   & pos->color[WHITE])) - (popcount(pos->piece[ROOK]   & pos->color[BLACK]))) +
           3 * ((popcount(pos->piece[BISHOP] & pos->color[WHITE])) - (popcount(pos->piece[BISHOP] & pos->color[BLACK]))) +
           9 * ((popcount(pos->piece[QUEEN]  & pos->color[WHITE])) - (popcount(pos->piece[QUEEN]  & pos->color[BLACK]))) +
         100 * ((popcount(pos->piece[KING]   & pos->color[WHITE])) - (popcount(pos->piece[KING]   & pos->color[BLACK]))) +
               ((popcount(pos->piece[PAWN]   & pos->color[WHITE])) - (popcount(pos->piece[PAWN]   & pos->color[BLACK])))
    );
}
uint16_t bestMove(struct Position *pos, struct Info info)
{
    unsigned int startTime = gettimems(), depth;
    for (depth = 0; gettimems() - startTime < 100; depth++)
    {
        search(pos, depth, 0, -MATE, MATE);
    }

    uint16_t ttMove; int ttEval, ttDepth, ttFlag;
    getTT(pos->state->zobrist, &ttMove, &ttEval, &ttDepth, &ttFlag);

    return ttMove;
}

int search(struct Position *pos, int depth, int height, int alpha, int beta)
{
    int alphaOrig = alpha;

    uint16_t ttMove; int ttEval, ttDepth, ttFlag;
    if (getTT(pos->state->zobrist, &ttMove, &ttEval, &ttDepth, &ttFlag) && ttDepth >= depth)
    {
        ttEval = evalFromTT(ttEval, height);

        if      (ttFlag == EXACT)                  return ttEval;
        else if (ttFlag == LOWERBOUND) alpha = MAX(alpha, ttEval);
        else if (ttFlag == UPPERBOUND) beta  = MIN(beta,  ttEval);

        if (alpha >= beta) return ttEval;
    }

    if (depth == 0) return evalPos(pos);

    uint16_t moveList[MAX_MOVES];
    uint16_t *begin = moveList;
    uint16_t *end   = generateLegalMoves(pos, moveList);
    if (begin == end)
    {
        // mate or stalemate
        return squareIsAttacked(__builtin_ctzll(pos->piece[KING] & pos->color[pos->state->turn]), !pos->state->turn, pos) ? -MATE + height : 0;
    }

    struct State newState;
    uint16_t bestMove; int eval, flag, nega;
    for (bestMove = *begin, eval = -MATE; begin < end; begin++)
    {
        doMove(*begin, pos, &newState);

        nega = -search(pos, depth - 1, height + 1, -beta, -alpha);

        eval  = MAX(eval, nega);

        if (eval > alpha) bestMove = *begin;

        alpha = MAX(eval, alpha);

        undoMove(*begin, pos);

        if (alpha >= beta) break;
    }

    if      (eval <= alphaOrig) flag = UPPERBOUND;
    else if (eval >= beta)      flag = LOWERBOUND;
    else                        flag = EXACT;
    putTT(pos->state->zobrist, bestMove, evalToTT(eval, height), depth, flag);

    return eval;
}
