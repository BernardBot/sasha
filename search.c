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

int history[PIECE_N][SQUARE_N] = {0};
int killers[2][MAX_MOVES] = {0};

const int MVVLVAVALUESVIC[] = 
{
    13000, 7000, 4750, 4500, 0, 1000,
    13000, 7000, 4750, 4500, 0, 1000,

    0
};
const int MVVLVAVALUESATT[] = 
{
    13, 7, 5, 4, 0, 1,
    13, 7, 5, 4, 0, 1,

    0
};
void evalNoisyMoves(struct Position *pos, uint16_t *moveList, int start, int end, int *evalList, int height)
{
    for (; start < end; start++)
    {
        if (moveList[start] == pvList[height])
        {
            evalList[start] = 100000;
        } else
        {
            int attPiece = pos->pieceType[ moveList[start]       & 0b111111];
            int vicPiece = pos->pieceType[(moveList[start] >> 6) & 0b111111];
            evalList[start] = MVVLVAVALUESVIC[vicPiece] - MVVLVAVALUESATT[attPiece];
        }
    }
}
void evalQuietMoves(struct Position *pos, uint16_t *moveList, int start, int end, int *evalList, int height)
{
    for (; start < end; start++)
    {
        const int ply = pos->state->halfmovecount;

        if (moveList[start] == pvList[height])
        {
            evalList[start] = 100000;
        }
        else if (moveList[start] == killers[0][ply])
        {
            evalList[start] = 90;
        } else if (moveList[start] == killers[1][ply])
        {
            evalList[start] = 80;
        } else
        {
            const int from  =  moveList[start]       & 0b111111;
            const int to    = (moveList[start] >> 6) & 0b111111;
            const int piece = pos->pieceType[from];

            evalList[start] = history[piece][to];
        }
    }
}

int getBestIndex(int *evalList, int size)
{
    int best = 0;
    for (int i = 1; i < size; i++)
    {
        if (evalList[i] > evalList[best])
        {
            best = i;
        }
    }
    return best;
}
uint16_t popMove(uint16_t *moveList, int *size, int *evalList, int index)
{
    uint16_t poppy = moveList[index];
    moveList[index] = moveList[--*size];
    evalList[index] = evalList[*size];
    return poppy;
}

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
unsigned int startTime;
uint16_t bestMove(struct Position *pos, struct Info info)
{
    startTime = gettimems(); int depth;
    uint16_t ttMove; int ttEval, ttDepth, ttFlag;

    for (depth = 1; gettimems() - startTime < uciInfo.movetime; depth++)
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
    if (gettimems() - startTime > uciInfo.movetime)
    {
        return 0;
    }

    nodes++;

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

    int evalList[MAX_MOVES];
    uint16_t moveList[MAX_MOVES];
    uint16_t *begin = moveList;
    uint16_t *end   = generateNoisyMoves(pos, moveList);
    int size = end - begin;
    evalNoisyMoves(pos, moveList, 0, size, evalList, 0);
    
    struct State newState;
    int i;
    while (size)
    {
        i = getBestIndex(evalList, size);

        doMove(moveList[i], pos, &newState);
        eval = -qsearch(pos, height + 1, -beta, -alpha);
        undoMove(moveList[i], pos);

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

        popMove(moveList, &size, evalList, i);
    }

    return best;
}

int search(struct Position *pos, int depth, int height, int alpha, int beta)
{
    if (gettimems() - startTime > uciInfo.movetime)
    {
        return 0;
    }

    nodes++;
    int alphaOrig = alpha;

    uint16_t ttMove; int ttEval, ttDepth, ttFlag, ttHit = 0;

    if (getTT(pos->state->zobrist, &ttMove, &ttEval, &ttDepth, &ttFlag) && ttDepth >= depth)
    {
        ttHit = 1;
        ttEval = evalFromTT(ttEval, height);

        if      (ttFlag == EXACT)                         return ttEval;
        else if (ttFlag == LOWERBOUND && ttEval > alpha) alpha = ttEval;
        else if (ttFlag == UPPERBOUND && ttEval < beta)  beta  = ttEval;

        if (alpha >= beta) 
        {
            return ttEval;
        }
    }

    int inCheck = squareIsAttacked(__builtin_ctzll(pos->piece[KING] & pos->color[pos->state->turn]), !pos->state->turn, pos);

    if (depth <= 0 && !inCheck)
    {
        return qsearch(pos, height, alpha, beta);
    }

    uint16_t moveList[MAX_MOVES];
    int evalList[MAX_MOVES];

    uint16_t *begin = moveList;

    uint16_t *noisyEnd = generateNoisyMoves(pos, moveList);
    evalNoisyMoves(pos, moveList, 0, noisyEnd - begin, evalList, height);

    uint16_t *quietEnd = generateQuietMoves(pos, noisyEnd);
    int size = quietEnd - begin;
    evalQuietMoves(pos, moveList, noisyEnd - begin, size, evalList, height);

    
    if (size == 0)
    {
        // mate or stalemate
        return inCheck ? -MATE + height : 0;
    }


    ttMove = moveList[0];
    ttEval = -MATE;

    struct State newState; 
    int eval;
    int i;
    while (size)
    {
        i = getBestIndex(evalList, size);
        
        doMove(moveList[i], pos, &newState);
        eval = -search(pos, depth - 1, height + 1, -beta, -alpha);
        undoMove(moveList[i], pos);

        if (eval > ttEval)
        {
            ttEval = eval;
            if (eval > alpha) 
            {
                alpha = eval;
                ttMove = pvList[height] = moveList[i];

                int from  =  moveList[i]       & 0b111111;
                int to    = (moveList[i] >> 6) & 0b111111;
                int piece = pos->pieceType[from];
                
                history[piece][to] += depth;
            }
        }

        if (alpha >= beta)
        {
            const int ply = pos->state->halfmovecount;

            killers[1][ply] = killers[0][ply];
            killers[0][ply] = moveList[i];

            break;
        }

        popMove(moveList, &size, evalList, i);
    }

    ttFlag = ttEval <= alphaOrig ? UPPERBOUND
           : ttEval >= beta      ? LOWERBOUND
           : EXACT;
    putTT(pos->state->zobrist, ttMove, evalToTT(ttEval, height), depth, ttFlag);

    return ttEval;
}
