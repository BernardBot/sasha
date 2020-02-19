#ifndef UCI_H
#define UCI_H

#include "position.h"
#include <sys/time.h>

#define FENSTART "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

struct Info
{
    int ponder, wtime, btime, binc, winc, movestogo, depth, nodes, mate, movetime, infinite;
    uint16_t searchmoves[MAX_MOVES];
};

struct Info uciInfo;

void initUciInfo();

void uciGo(char *s, struct Position *pos);
void uciPos(char *s, struct Position *pos, struct State stateList[]);
void uciLoop(struct Position *pos, struct State stateList[]);

unsigned int gettimems();

#endif
