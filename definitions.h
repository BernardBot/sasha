#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <stdint.h>

enum Square
{
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1,

    SQUARE_N, NO_SQ,
};

enum Color { WHITE, BLACK, COLOR_N, NO_COLOR, };
enum Piece { QUEEN, ROOK, BISHOP, KNIGHT, KING, PAWN, PIECE_N, NO_PIECE, };
enum PieceType
{ 
    WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT, WHITE_KING, WHITE_PAWN,
    BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT, BLACK_KING, BLACK_PAWN,

    EMPTY, PIECETYPE_N, NO_PIECETYPE,
};
enum Castle { WOO = 1, WOOO = 2, BOO = 4, BOOO = 8, CASTLE_N = 16,};
enum Tag { CASTLING = 1, PROMOTION = 2, ENPASSANT = 3, };

enum { MAX_PLY = 128, MAX_MOVES = 256, };
enum {
    MATE = 32000,
    MATE_IN_MAX = MATE - MAX_PLY,
    MATED_IN_MAX = MAX_PLY - MATE,
};
enum {
    FILE_N = 8,
    RANK_N = 8,
};

const uint64_t FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH;
const uint64_t RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8;

const uint64_t FILES[FILE_N];
const uint64_t RANKS[RANK_N];

#define rank_bb(sq) RANKS[7 - ((sq) >> 3)]
#define file_bb(sq) FILES[(sq) & 7]
#define shift_bb(bb, s) ((s) < 0 ? (bb) >> -(s) : (bb) << (s))
#define sq_bb(sq) (1ull << (sq))

const uint64_t NILEA, NILEH;
const uint64_t NILEAB, NILEGH;

const int SHIFT[FILE_N];
const uint64_t WRAP[FILE_N];

const enum Castle OO[COLOR_N];
const enum Castle OOO[COLOR_N];

const uint64_t OO_MASK[COLOR_N];
const uint64_t OOO_MASK[COLOR_N];

const int BRANK[COLOR_N][FILE_N];

const char *squareString[SQUARE_N];
const char pieceTypeChar[PIECETYPE_N];

// search lookup tables
const int PIECEVALUE [PIECE_N];
const int PIECESQUARE[PIECE_N][SQUARE_N];

#endif // DEFINITIONS_H_
