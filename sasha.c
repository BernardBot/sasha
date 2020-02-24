#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "attacks.h"
#include "definitions.h"
#include "lookup.h"
#include "position.h"
#include "movegen.h"
#include "position.h"
#include "uci.h"
#include "hashtable.h"
#include "search.h"

int main()
{
    initLookup();
    initUciInfo();

    struct Position pos = {0};
    struct State firstState = {0};
    struct State stateList[MAX_PLY];
    pos.state = &firstState;

    parseFen(FENSTART, &pos);
    
    uciLoop(&pos, stateList);

    return 0;
}
