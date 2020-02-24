#include "definitions.h"
#include <stdint.h>

const uint64_t FILEA = 0x0101010101010101ull;
const uint64_t FILEB = FILEA << 1;
const uint64_t FILEC = FILEA << 2;
const uint64_t FILED = FILEA << 3;
const uint64_t FILEE = FILEA << 4;
const uint64_t FILEF = FILEA << 5;
const uint64_t FILEG = FILEA << 6;
const uint64_t FILEH = FILEA << 7;

const uint64_t RANK1 = 0xff00000000000000ull;
const uint64_t RANK2 = RANK1 >> 8;
const uint64_t RANK3 = RANK1 >> 16;
const uint64_t RANK4 = RANK1 >> 24;
const uint64_t RANK5 = RANK1 >> 32;
const uint64_t RANK6 = RANK1 >> 40;
const uint64_t RANK7 = RANK1 >> 48;
const uint64_t RANK8 = RANK1 >> 56;

const uint64_t FILES[8] = { FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH };
const uint64_t RANKS[8] = { RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8 };

const uint64_t NILEA = ~FILEA;
const uint64_t NILEH = ~FILEH;

const uint64_t NILEAB = ~(FILEA | FILEB);
const uint64_t NILEGH = ~(FILEG | FILEH);

const int SHIFT[8] = {9, 1, -7, -8, -9, -1, 7, 8};
const uint64_t WRAP[8] = {
    0xfefefefefefefe00ull,
    0xfefefefefefefefeull,
    0x00fefefefefefefeull,
    0x00ffffffffffffffull,
    0x007f7f7f7f7f7f7full,
    0x7f7f7f7f7f7f7f7full,
    0x7f7f7f7f7f7f7f00ull,
    0xffffffffffffff00ull,
};

const enum Castle OO[COLOR_N] = { WOO, BOO, };
const enum Castle OOO[COLOR_N] = { WOOO, BOOO, };

const uint64_t OO_MASK[COLOR_N] = { 0x6000000000000000ull, 0x60ull, };
const uint64_t OOO_MASK[COLOR_N] = { 0xe00000000000000ull, 0xeull, };

const int BRANK[COLOR_N][8] = 
{
    { A1, B1, C1, D1, E1, F1, G1, H1, },
    { A8, B8, C8, D8, E8, F8, G8, H8, },
};

const char *squareString[SQUARE_N] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};
const char pieceTypeChar[PIECETYPE_N] = "QRBNKPqrbnkp.";

// QUEEN, ROOK, BISHOP, KNIGHT, KING, PAWN
const int PIECEVALUE[PIECE_N] = { 900, 500, 330, 320, MATE, 100 };

const int PIECESQUARE[PIECE_N][SQUARE_N] =
{
    {
-20,-10,-10, -5, -5,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5,  5,  5,  5,  0,-10,
 -5,  0,  5,  5,  5,  5,  0, -5,
  0,  0,  5,  5,  5,  5,  0, -5,
-10,  5,  5,  5,  5,  5,  0,-10,
-10,  0,  5,  0,  0,  0,  0,-10,
-20,-10,-10, -5, -5,-10,-10,-20
    },
    {
  0,  0,  0,  0,  0,  0,  0,  0,
  5, 10, 10, 10, 10, 10, 10,  5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  0,  0,  0,  5,  5,  0,  0,  0        
    },
    {
-20,-10,-10,-10,-10,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5, 10, 10,  5,  0,-10,
-10,  5,  5, 10, 10,  5,  5,-10,
-10,  0, 10, 10, 10, 10,  0,-10,
-10, 10, 10, 10, 10, 10, 10,-10,
-10,  5,  0,  0,  0,  0,  5,-10,
-20,-10,-10,-10,-10,-10,-10,-20        
    },
    {
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  0,  0,  0,-20,-40,
-30,  0, 10, 15, 15, 10,  0,-30,
-30,  5, 15, 20, 20, 15,  5,-30,
-30,  0, 15, 20, 20, 15,  0,-30,
-30,  5, 10, 15, 15, 10,  5,-30,
-40,-20,  0,  5,  5,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50        
    },
    {
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-20,-30,-30,-40,-40,-30,-30,-20,
-10,-20,-20,-20,-20,-20,-20,-10,
 20, 20,  0,  0,  0,  0, 20, 20,
 20, 30, 10,  0,  0, 10, 30, 20
    },
    {
 0,  0,  0,  0,  0,  0,  0,  0,
50, 50, 50, 50, 50, 50, 50, 50,
10, 10, 20, 30, 30, 20, 10, 10,
 5,  5, 10, 25, 25, 10,  5,  5,
 0,  0,  0, 20, 20,  0,  0,  0,
 5, -5,-10,  0,  0,-10, -5,  5,
 5, 10, 10,-20,-20, 10, 10,  5,
 0,  0,  0,  0,  0,  0,  0,  0        
    }
};
