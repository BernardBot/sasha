#ifndef PERFT_H
#define PERFT_H

#include <stdint.h>
#include "position.h"

uint64_t perft(int depth, struct Position *pos);

#endif
