#include <stdio.h>
#include "util.h"
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "lookup.h"
#include "definitions.h"
#include "hashtable.h"
#include <stdlib.h>
#include "uci.h"

uint16_t pvList[MAX_PLY];
int nodes;

int evalPos(struct Position *pos)
{
    int sqEval, sq, pieceType, pieceColor, piece;
    for (sqEval = sq = 0; sq < SQUARE_N; sq++)
    {
        pieceType  = pos->pieceType[sq];
        if (pieceType != EMPTY)
        {
            piece      = pieceType %  PIECE_N;
            pieceColor = pieceType >= PIECE_N;
            if (pieceColor == WHITE)
            {
                sqEval += PIECESQUARE[piece][     sq];
            } else
            {
                sqEval -= PIECESQUARE[piece][63 - sq];
            }
        }        
    }

    return (1 - 2 * pos->state->turn) * 
    (
           sqEval +
           PIECEVALUE[PAWN]   * ((popcount(pos->piece[PAWN]   & pos->color[WHITE])) - (popcount(pos->piece[PAWN]   & pos->color[BLACK]))) +
           PIECEVALUE[KNIGHT] * ((popcount(pos->piece[KNIGHT] & pos->color[WHITE])) - (popcount(pos->piece[KNIGHT] & pos->color[BLACK]))) +
           PIECEVALUE[BISHOP] * ((popcount(pos->piece[BISHOP] & pos->color[WHITE])) - (popcount(pos->piece[BISHOP] & pos->color[BLACK]))) +
           PIECEVALUE[ROOK]   * ((popcount(pos->piece[ROOK]   & pos->color[WHITE])) - (popcount(pos->piece[ROOK]   & pos->color[BLACK]))) +
           PIECEVALUE[QUEEN]  * ((popcount(pos->piece[QUEEN]  & pos->color[WHITE])) - (popcount(pos->piece[QUEEN]  & pos->color[BLACK])))
    );
}
uint16_t bestMove(struct Position *pos, struct Info info)
{
    unsigned int startTime = gettimems(), depth;
    uint16_t ttMove; int ttEval, ttDepth, ttFlag;

    for (depth = 1; gettimems() - startTime < 100; depth++)
    {
        nodes = 0;
        search(pos, depth, 0, -MATE, MATE);

        getTT(pos->state->zobrist, &ttMove, &ttEval, &ttDepth, &ttFlag);

        printf("info depth %d seldepth %d score cp %d nodes %d time %d ", depth, depth, ttEval, nodes, gettimems() - startTime);

        printf("pv");
        for (int i = 0; i < depth; i++)
        {
            printf(" "); printMove(pvList[i]);
        }
        printf("\n");

        fflush(stdout); // don't forget to flush
    }


    return ttMove;
}

int qsearch(struct Position *pos, int height, int alpha, int beta)
{
    int eval = evalPos(pos);

    if (eval >= beta)
    {
        return eval;
    }

    if (eval > alpha)
    {
        alpha = eval;
    }

    int best = eval;

    uint16_t moveList[MAX_MOVES];
    uint16_t *begin = moveList;
    uint16_t *end   = generateNoisyMoves(pos, moveList);
    
    struct State newState;
    for ( ; begin < end; begin++)
    {
        doMove(*begin, pos, &newState);
        eval = -qsearch(pos, height + 1, -beta, -alpha);
        undoMove(*begin, pos);

        if (eval > best)
        {
            best = eval;
            if (eval > alpha) 
            {
                alpha = eval;
            }
        }

        if (alpha >= beta)
        {
            break;
        }
    }

    return best;
}

int search(struct Position *pos, int depth, int height, int alpha, int beta)
{
    int alphaOrig = alpha;

    uint16_t ttMove; int ttEval, ttDepth, ttFlag;

    if (getTT(pos->state->zobrist, &ttMove, &ttEval, &ttDepth, &ttFlag) && ttDepth >= depth)
    {
        ttEval = evalFromTT(ttEval, height);

        if      (ttFlag == EXACT)                         return ttEval;
        else if (ttFlag == LOWERBOUND && ttEval > alpha) alpha = ttEval;
        else if (ttFlag == UPPERBOUND && ttEval < beta)  beta  = ttEval;

        if (alpha >= beta) 
        {
            return ttEval;
        }
    }

    if (depth == 0)
    {
        nodes++;
        return qsearch(pos, height, alpha, beta);//evalPos(pos);
    }

    uint16_t moveList[MAX_MOVES];
    uint16_t *begin = moveList;
    uint16_t *end   = generateLegalMoves(pos, moveList);
    
    if (begin == end)
    {
        // mate or stalemate
        return squareIsAttacked(__builtin_ctzll(pos->piece[KING] & pos->color[pos->state->turn]), !pos->state->turn, pos) ? -MATE + height : 0;
    }

    struct State newState; int eval;
    for (ttMove = *begin, ttEval = -MATE; begin < end; begin++)
    {
        doMove(*begin, pos, &newState);
        eval = -search(pos, depth - 1, height + 1, -beta, -alpha);
        undoMove(*begin, pos);

        if (eval > ttEval)
        {
            ttEval = eval;
            if (eval > alpha) 
            {
                alpha = eval;
                ttMove = pvList[height] = *begin;
            }
        }

        if (alpha >= beta)
        {
            break;
        }
    }

    ttFlag = ttEval <= alphaOrig ? UPPERBOUND
           : ttEval >= beta      ? LOWERBOUND
           : EXACT;
    putTT(pos->state->zobrist, ttMove, evalToTT(ttEval, height), depth, ttFlag);

    return ttEval;
}
