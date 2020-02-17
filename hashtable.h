#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>

#define TABLESIZE 65536 // 2 ^ 16
#define TABLESHIFT 48 // 64 - 16

struct Entry {
    uint64_t key;
    uint16_t move;
    int eval;
};

struct Entry TTABLE[TABLESIZE];

int  getTT(uint64_t key, uint16_t *move, int *eval);
void putTT(uint64_t key, uint16_t  move, int  eval);

#endif
