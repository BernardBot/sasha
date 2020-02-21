#include <stdio.h>
#include "util.h"
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "lookup.h"
#include "hashtable.h"
#include <stdlib.h>
#include "uci.h"

uint16_t pvList[MAX_PLY];

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
                sqEval -= PIECESQUARE[piece][64 - sq];
            }
        }        
    }

    return (1 - 2 * pos->state->turn) * 
    (
           sqEval +
           100 * ((popcount(pos->piece[PAWN]   & pos->color[WHITE])) - (popcount(pos->piece[PAWN]   & pos->color[BLACK]))) +
           320 * ((popcount(pos->piece[KNIGHT] & pos->color[WHITE])) - (popcount(pos->piece[KNIGHT] & pos->color[BLACK]))) +
           330 * ((popcount(pos->piece[BISHOP] & pos->color[WHITE])) - (popcount(pos->piece[BISHOP] & pos->color[BLACK]))) +
           500 * ((popcount(pos->piece[ROOK]   & pos->color[WHITE])) - (popcount(pos->piece[ROOK]   & pos->color[BLACK]))) +
           900 * ((popcount(pos->piece[QUEEN]  & pos->color[WHITE])) - (popcount(pos->piece[QUEEN]  & pos->color[BLACK])))
    );
}
uint16_t bestMove(struct Position *pos, struct Info info)
{
    unsigned int startTime = gettimems(), depth;
    uint16_t ttMove; int ttEval, ttDepth, ttFlag;

    for (depth = 1; gettimems() - startTime < 100; depth++)
    {
        search(pos, depth, 0, -MATE, MATE);

        getTT(pos->state->zobrist, &ttMove, &ttEval, &ttDepth, &ttFlag);

        printf("info depth %d seldepth %d score cp %d nodes 0 time %d ", depth, depth, ttEval, gettimems() - startTime);

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
        return evalPos(pos);
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

int PIECESQUARE[PIECE_N][SQUARE_N] = 
{
    {
-20,-10,-10, -5, -5,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5,  5,  5,  5,  0,-10,
 -5,  0,  5,  5,  5,  5,  0, -5,
  0,  0,  5,  5,  5,  5,  0, -5,
-10,  5,  5,  5,  5,  5,  0,-10,
-10,  0,  5,  0,  0,  0,  0,-10,
-20,-10,-10, -5, -5,-10,-10,-20
    },
    {
  0,  0,  0,  0,  0,  0,  0,  0,
  5, 10, 10, 10, 10, 10, 10,  5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  0,  0,  0,  5,  5,  0,  0,  0        
    },
    {
-20,-10,-10,-10,-10,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5, 10, 10,  5,  0,-10,
-10,  5,  5, 10, 10,  5,  5,-10,
-10,  0, 10, 10, 10, 10,  0,-10,
-10, 10, 10, 10, 10, 10, 10,-10,
-10,  5,  0,  0,  0,  0,  5,-10,
-20,-10,-10,-10,-10,-10,-10,-20        
    },
    {
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  0,  0,  0,-20,-40,
-30,  0, 10, 15, 15, 10,  0,-30,
-30,  5, 15, 20, 20, 15,  5,-30,
-30,  0, 15, 20, 20, 15,  0,-30,
-30,  5, 10, 15, 15, 10,  5,-30,
-40,-20,  0,  5,  5,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50        
    },
    {
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-20,-30,-30,-40,-40,-30,-30,-20,
-10,-20,-20,-20,-20,-20,-20,-10,
 20, 20,  0,  0,  0,  0, 20, 20,
 20, 30, 10,  0,  0, 10, 30, 20
    },
    {
 0,  0,  0,  0,  0,  0,  0,  0,
50, 50, 50, 50, 50, 50, 50, 50,
10, 10, 20, 30, 30, 20, 10, 10,
 5,  5, 10, 25, 25, 10,  5,  5,
 0,  0,  0, 20, 20,  0,  0,  0,
 5, -5,-10,  0,  0,-10, -5,  5,
 5, 10, 10,-20,-20, 10, 10,  5,
 0,  0,  0,  0,  0,  0,  0,  0        
    }
};
