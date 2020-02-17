#ifndef UCI_H
#define UCI_H

#include "position.h"

#define FENSTART "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void uciGo(char *s, struct Position *pos);
void uciPos(char *s, struct Position *pos, struct State stateList[]);
void uciLoop(struct Position *pos, struct State stateList[]);

#endif
