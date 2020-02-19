#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>

#define TABLESIZE 65536 // 2 ^ 16
#define TABLESHIFT 48 // 64 - 16

enum Flag {EXACT, LOWERBOUND, UPPERBOUND};

struct Entry {
    uint64_t key;
    uint16_t move;
    int eval;
    int depth;
    int flag;
};

struct Entry TTABLE[TABLESIZE];

int  getTT(uint64_t key, uint16_t *move, int *eval, int *depth, int *flag);
void putTT(uint64_t key, uint16_t  move, int  eval, int  depth, int  flag);

int evalFromTT(int eval, int height);
int evalToTT  (int eval, int height);

#endif
