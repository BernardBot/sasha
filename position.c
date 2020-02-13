#include <stdint.h>
#include <string.h>
#include "definitions.h"
#include "position.h"
#include "util.h"

int pieceOf(int pieceType)
{
    return pieceType == EMPTY ? -1 : pieceType & 0b1111;
}
int colorOf(int pieceType)
{
    return pieceType == EMPTY ? -1 : pieceType > 15;
}

void doMove(uint16_t move, struct Position *pos, struct State *newState)
{
    // update boards and state
    const int from =  move        & 0b111111;
    const int to   = (move >> 6)  & 0b111111;
    const int tag  = (move >> 12) & 0b11;

    // assume there is a piece on the from index square
    const int fromType  = pos->pieceType[from];
          int toType    = pos->pieceType[to];

    const int fromPiece = pieceOf(fromType);
    const int fromColor = colorOf(fromType);

    if (tag == ENPASSANT)
    {
        toType = fromColor == WHITE ? WHITE_PAWN : BLACK_PAWN;
    }

    const uint64_t fromToSquare = sq_bb(from) | sq_bb(to);

    // move piece
    pos->piece[fromPiece] ^= fromToSquare;
    pos->color[fromColor] ^= fromToSquare;
    pos->pieceType[from]   = EMPTY;
    pos->pieceType[to]     = fromType;

    newState->movecount         = pos->state->movecount + pos->state->turn;
    newState->halfmovecount     = pos->state->halfmovecount + 1;
    newState->turn              = !pos->state->turn; // watch out the color is updated
    // reset stuff
    newState->castling          = pos->state->castling;
    newState->enpassant         = -1;
    newState->capturedSquare    = -1;
    newState->capturedPieceType = -1;

    newState->previousState        = pos->state;
    pos->state                     = newState;


    if (toType != EMPTY) // a piece is captured
    {
        if (tag == ENPASSANT)
        {
            const int epSq = fromColor == WHITE ? to + 8 : to - 8;
            const uint64_t epSquare = sq_bb(epSq);

            pos->piece[PAWN]       ^= epSquare;
            pos->color[!fromColor] ^= epSquare;
            pos->pieceType[epSq] = EMPTY;

            pos->state->capturedSquare = epSq;
            pos->state->capturedPieceType = toType;
        } else
        {
            const int toPiece = pieceOf(toType);
            const int toSquare = sq_bb(to);

            pos->piece[toPiece]    ^= toSquare;
            pos->color[!fromColor] ^= toSquare;
            // pos->pieceType[to] = EMPTY; already overwritten by moving the piece

            pos->state->capturedSquare = to;
            pos->state->capturedPieceType = toType;
        }
    }

    if (tag == CASTLING)
    {
        // we have to move the king (rook already done)
        const int kingFrom = to > from ? to + 1 : to - 1;
        const int kingTo   = to > from ? to - 1 : to + 1;
        const uint64_t kingFromToSquare = sq_bb(kingFrom) | sq_bb(kingTo);

        pos->piece[KING]        ^= kingFromToSquare;
        pos->color[fromColor]   ^= kingFromToSquare;
        pos->pieceType[kingFrom] = EMPTY;
        pos->pieceType[kingTo]   = fromColor == WHITE ? WHITE_KING : BLACK_KING;
        
    } else if (fromPiece == PAWN)
    {
        if (to - from == 16 || to - from == -16)
        {
            pos->state->enpassant = fromColor == WHITE ? to + 8 : to - 8;
        } else if (tag == PROMOTION)
        {
            const int promPiece = move >> 14;
            const uint64_t toSquare = sq_bb(to);

            pos->piece[fromPiece] ^= toSquare;
            pos->piece[promPiece] ^= toSquare;
            pos->pieceType[to]     = fromColor == WHITE ? promPiece : promPiece + 16;
        }
    }

    // Another day, another death, another dollar
    // update castling rights
}

void undoMove(uint16_t move, struct Position *pos)
{

}

void readFen(char *fen, struct Position *pos)
{
    int i, j, color, piece, pieceType;
    
    for (i = 0; i < SQUARE_N && *fen != ' '; fen++)
    {
        switch (*fen)
        {
        case 'R': color = WHITE; piece = ROOK;   pieceType = WHITE_ROOK;   break;
        case 'r': color = BLACK; piece = ROOK;   pieceType = BLACK_ROOK;   break;
        case 'N': color = WHITE; piece = KNIGHT; pieceType = WHITE_KNIGHT; break;
        case 'n': color = BLACK; piece = KNIGHT; pieceType = BLACK_KNIGHT; break;
        case 'B': color = WHITE; piece = BISHOP; pieceType = WHITE_BISHOP; break;
        case 'b': color = BLACK; piece = BISHOP; pieceType = BLACK_BISHOP; break;
        case 'Q': color = WHITE; piece = QUEEN;  pieceType = WHITE_QUEEN;  break;
        case 'q': color = BLACK; piece = QUEEN;  pieceType = BLACK_QUEEN;  break;
        case 'K': color = WHITE; piece = KING;   pieceType = WHITE_KING;   break;
        case 'k': color = BLACK; piece = KING;   pieceType = BLACK_KING;   break;
        case 'P': color = WHITE; piece = PAWN;   pieceType = WHITE_PAWN;   break;
        case 'p': color = BLACK; piece = PAWN;   pieceType = BLACK_PAWN;   break;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
                  color = -1;    piece = -1;     pieceType = EMPTY;        break;
        case '/': color = -1;    piece = -1;     pieceType = -1;           break;
        default:                                                           break;
        }

        if (piece != -1 && color != -1 && pieceType != -1)
        {
            pos->color[color] |= sq_bb(i);
            pos->piece[piece] |= sq_bb(i);
            pos->pieceType[i++] = pieceType;
        }
        if (pieceType == EMPTY)
        {
            for (j = i + *fen - '0'; i < j; i++)
            {
                pos->pieceType[i] = pieceType;
            }
        }
    }

    fen++;

    if (*fen == 'w')
    {
        pos->state->turn = WHITE;
    } else if (*fen == 'b')
    {
        pos->state->turn = BLACK;
    }

    fen++;
    fen++;

    for (pos->state->castling = 0; *fen != ' '; fen++)
    {
        if      (*fen == 'K') pos->state->castling |= WOO;
        else if (*fen == 'Q') pos->state->castling |= WOOO;
        else if (*fen == 'k') pos->state->castling |= BOO;
        else if (*fen == 'q') pos->state->castling |= BOOO;
    }

    fen++;

    if (*fen == '-')
    {
        pos->state->enpassant = -1;
    } else
    {
        char f = *fen++;
        char r = *fen;

        pos->state->enpassant = (f - 'a') + (7 - (r - '1')) * 8;
    }

    fen++;
    fen++;

    for (i = 0; *fen != ' '; fen++, i *= 10)
    {
        i += *fen - '0';
    }
    pos->state->movecount = i / 10;

    fen++;

    for (i = 0; *fen; fen++, i *= 10)
    {
        i += *fen - '0';
    }
    pos->state->halfmovecount = i / 10;
}
