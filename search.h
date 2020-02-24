#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>
#include "position.h"
#include "uci.h"
#include "definitions.h"

int evalPos(struct Position *pos);
uint16_t bestMove(struct Position *pos, struct Info info);
int search(struct Position *pos, int depth, int height, int alpha, int beta);

#endif
