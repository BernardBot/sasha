#include "uci.h"
#include "movegen.h"
#include "position.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// use strstr and strncmp
void uciGo(char *s, struct Position *pos)
{
    int a = 1;
}
void uciPos(char *s, struct Position *pos, struct State stateList[])
{
    if (strncmp(s, "position", 8)) return;
    while (*s && *s++ != ' ') ;

    if (0 == strcmp(s, "startpos"))
    {
        parseFen(FENSTART, pos);
        return;
    }

    if (strncmp(s, "fen",      3)) return;
    while (*s && *s++ != ' ') ;

    // pointer can go to far if movecount / halfmovecount are not specified
    s = parseFen(s, pos);

    if (strncmp(s, "moves",    5)) return;
    while (*s && *s++ != ' ') ;

    uint16_t legalList[256];
    uint16_t *legalBegin;
    uint16_t *legalEnd;
    uint16_t move;

    char f0, r0, f1, r1;
    int from, to, tag, prom;
    int stateP = 0;
    while (*s)
    {
        if ('h' < *s && *s < 'a') return; f0 = *s++;
        if ('8' < *s && *s < '1') return; r0 = *s++;
        if ('h' < *s && *s < 'a') return; f1 = *s++;
        if ('8' < *s && *s < '1') return; r1 = *s++;

        from = parseSquare(f0, r0);
        to   = parseSquare(f1, r1);

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
        } else
        {
            tag = 0;
            prom = 0;
        }
        
        move = (from) | (to << 6) | (tag << 12) | (prom << 14);

        legalEnd = generateLegalMoves(pos, legalList);
        // printf("%lu\n", legalEnd - legalBegin);
        for (legalBegin = legalList; legalBegin < legalEnd; legalBegin++)
        {
            printMove(*legalBegin);
            if (move == *legalBegin)
            {
                break;
            }
            else if (move == (*legalBegin & 0xfff))
            {
                move = *legalBegin;
                break;
            }
        }
        doMove(move, pos, &stateList[stateP++]);
    }

}
void uciLoop(struct Position *pos, struct State stateList[])
{
    char s[8192];

    while (scanf("%s", s))
    {
        if        (0 == strcmp(s, "uci"))
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
        } else if (0 == strcmp(s, "ucinewgame"))
        {
            uciPos("position startpos", pos, stateList);
        } else if (0 == strncmp(s, "go", 2))
        {
            uciGo(s, pos);
        } else if (0 == strcmp(s, "quit"))
        {
            return;
        }
    }
}
