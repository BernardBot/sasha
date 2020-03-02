#include "uci.h"
#include "movegen.h"
#include "position.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "search.h"

void initUciInfo()
{
    uciInfo.ponder    = 0;
    uciInfo.wtime     = 0;
    uciInfo.btime     = 0;
    uciInfo.binc      = 0;
    uciInfo.winc      = 0;
    uciInfo.movestogo = 0;
    uciInfo.depth     = 5;
    uciInfo.nodes     = 0;
    uciInfo.mate      = 0;
    uciInfo.movetime  = 1000;
    uciInfo.infinite  = 0;
}

// use strstr and strncmp
void uciGo(char *s, struct Position *pos)
{
    uint16_t bMove;

    if (strncmp(s, "go", 2)) return;
    while (*s && *s++ != ' ') ;

    // parse input
    while (*s)
    {
        if        (0 == strncmp(s, "searchmoves", 11))
        {
           while (*s && *s++ != ' ') ;
            // TODO: parse all the moves
        } else if (0 == strncmp(s, "ponder", 6))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.ponder = 1;
        } else if (0 == strncmp(s, "wtime", 5))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.wtime = atoi(s);
        } else if (0 == strncmp(s, "btime", 5))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.btime = atoi(s);
        } else if (0 == strncmp(s, "binc", 4))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.binc = atoi(s);
        } else if (0 == strncmp(s, "winc", 4))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.winc = atoi(s);
        } else if (0 == strncmp(s, "movestogo", 9))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.movestogo = atoi(s);
        } else if (0 == strncmp(s, "depth", 5))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.depth = atoi(s);
        } else if (0 == strncmp(s, "nodes", 5))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.nodes = atoi(s);
        } else if (0 == strncmp(s, "mate", 4))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.mate = 1;
        } else if (0 == strncmp(s, "movetime", 8))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.movetime = atoi(s) - 100;
        } else if (0 == strncmp(s, "infinite", 8))
        {
            while (*s && *s++ != ' ') ;
            uciInfo.infinite = 1;
        }
        while (*s && *s++ != ' ') ;
    }

    bMove = bestMove(pos, uciInfo);

    printf("bestmove "); printMove(bMove); 

    if (((bMove >> 12) & 0b11) == PROMOTION)
    {
        int prom = (bMove >> 14) & 0b11;
        if (prom == QUEEN)  printf("q");
        if (prom == ROOK)   printf("r");
        if (prom == KNIGHT) printf("n");
        if (prom == BISHOP) printf("b");
    }

    printf("\n");
}
void uciPos(char *s, struct Position *pos, struct State stateList[])
{
    // we need to reset the position first
    if (strncmp(s, "position", 8)) return;
    while (*s && *s++ != ' ') ;

    if (0 == strncmp(s, "startpos", 8))
    {
        parseFen(FENSTART, pos);

    } else if (0 == strncmp(s, "fen", 3))
    {
        while (*s && *s++ != ' ') ;
        s = parseFen(s, pos); // needs to clear the board
    }
    while (*s && *s++ != ' ') ;

    if (strncmp(s, "moves",    5)) return;
    while (*s && *s++ != ' ') ;

    uint16_t legalList[MAX_MOVES];
    uint16_t *legalBegin;
    uint16_t *legalEnd;
    uint16_t move;

    char f0, r0, f1, r1;
    int from, to, tag, prom;
    int stateP = 0;
    int legal;
    while (*s)
    {
        if ('h' < *s && *s < 'a') return; f0 = *s++;
        if ('8' < *s && *s < '1') return; r0 = *s++;
        if ('h' < *s && *s < 'a') return; f1 = *s++;
        if ('8' < *s && *s < '1') return; r1 = *s++;

        from = parseSquare(f0, r0);
        to   = parseSquare(f1, r1);
        tag  = 0;
        prom = 0;

        if (*s == 'q' || *s == 'r' || *s == 'n' || *s == 'b')
        {
            tag  = PROMOTION;
            switch(*s++)
            {
                case 'q': prom = QUEEN;  break;
                case 'r': prom = ROOK;   break;
                case 'n': prom = KNIGHT; break;
                case 'b': prom = BISHOP; break;
                default:                 break;
            }
        }
        
        move = (from) | (to << 6) | (tag << 12) | (prom << 14);

        legalBegin = legalList;
        legalEnd   = generateLegalMoves(pos, legalList);

        for (legal = 0; legalBegin < legalEnd; legalBegin++)
        {
            if (move == *legalBegin || move == (*legalBegin & 0xfff)) // this adds the castling / enpassant tag
            {
                move = *legalBegin;
                legal = 1;
                break;
            }
        }

        if (legal)
        {
            doMove(move, pos, &stateList[stateP++]);
        } else
        {
            return; //illegal move;
        }

        // skip whitespace
        while (*s == ' ') s++;
    }

}
void uciLoop(struct Position *pos, struct State stateList[])
{
    char s[8192];
    
    while (fgets(s, 8192, stdin)) // scanf is bad, splits at whitespace
    {
        if        (0 == strncmp(s, "ucinewgame", 10))
        {
            uciPos("position startpos", pos, stateList);
        } else if (0 == strncmp(s, "uci", 3))
        {
            printf("id name Sasha\n");
            printf("id author Bernard Bot\n");
            printf("uciok\n");
        } else if (0 == strncmp(s, "isready", 7))
        {
            printf("readyok\n");
        } else if (0 == strncmp(s, "position", 8))
        {
            uciPos(s, pos, stateList);
        } else if (0 == strncmp(s, "go", 2))
        {
            uciGo(s, pos);
        } else if (0 == strncmp(s, "quit", 4))
        {
            return;
        }
        fflush(stdout); // don't forget to flush
    }
}

unsigned int gettimems()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}
