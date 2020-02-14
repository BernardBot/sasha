#include <stdint.h>
#include "definitions.h"
#include "position.h"

void doMove(uint16_t move, struct Position *pos, struct State *newState)
{
    // parse move
    const int from =  move        & 0b111111;
    const int to   = (move >> 6)  & 0b111111;
    const int tag  = (move >> 12) & 0b11;

    // assume there is a piece on the from index square
    const int fromType  = pos->pieceType[from];
    const int toType    = pos->pieceType[to]; // this one can be empty

    // convert pieceType to piece and color
    const int fromPiece = fromType  % PIECE_N;
    const int fromColor = fromType >= PIECE_N;

    const uint64_t fromToSquare = sq_bb(from) | sq_bb(to);

    // initialize new state
    newState->movecount         = pos->state->movecount     + pos->state->turn;
    newState->halfmovecount     = pos->state->halfmovecount + 1;
    newState->castling          = pos->state->castling;
    newState->enpassant         = -1;
    newState->capturedSquare    = -1;
    newState->capturedPieceType = -1;
    // maybe do this later?
    newState->turn              = !pos->state->turn; // watch out the color is updated
    newState->previousState     = pos->state;
    pos->state                  = newState;

    // move piece
    pos->piece[fromPiece] ^= fromToSquare;
    pos->color[fromColor] ^= fromToSquare;
    pos->pieceType[from]   = EMPTY;
    pos->pieceType[to]     = fromType;

    if (toType != EMPTY) // a piece is captured
    {
        const int toPiece  = fromType % PIECE_N;
        const int toSquare = sq_bb(to);

        pos->piece[toPiece]    ^= toSquare;
        pos->color[!fromColor] ^= toSquare;
        // pos->pieceType[to]   = EMPTY; already overwritten by moving the piece

        pos->state->capturedSquare    = to;
        pos->state->capturedPieceType = toType;
    } else if (tag == CASTLING)
    {
        // we have to move the king (rook already done)
        const int kingType = fromColor * PIECE_N + KING;
        const int kingFrom = to > from ? to + 1 : to - 1;
        const int kingTo   = to > from ? to - 1 : to + 1;
        const uint64_t kingFromToSquare = sq_bb(kingFrom) | sq_bb(kingTo);

        pos->piece[KING]        ^= kingFromToSquare;
        pos->color[fromColor]   ^= kingFromToSquare;
        pos->pieceType[kingFrom] = EMPTY;
        pos->pieceType[kingTo]   = kingType;
    }

    if (fromPiece == PAWN)
    {
        if (tag == ENPASSANT)
        {
            const int epType = (!fromColor) * PIECE_N + PAWN;
            const int epSq   = fromColor == WHITE ? to + 8 : to - 8;
            const uint64_t epSquare = sq_bb(epSq);

            pos->piece[PAWN]       ^= epSquare;
            pos->color[!fromColor] ^= epSquare;
            pos->pieceType[epSq]    = EMPTY;

            pos->state->capturedSquare    = epSq;
            pos->state->capturedPieceType = epType;
        } else if (tag == PROMOTION)
        {
            const int promPiece = move >> 14;
            const int promType  = fromColor * PIECE_N + promPiece;
            const uint64_t toSquare = sq_bb(to);

            pos->piece[PAWN]      ^= toSquare;
            pos->piece[promPiece] ^= toSquare;
            pos->pieceType[to]     = promType;
        } else if (to - from == 16 || to - from == -16)
        {
            pos->state->enpassant = fromColor == WHITE ? to + 8 : to - 8;
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
        case 'R': pieceType = WHITE_ROOK;   break;
        case 'r': pieceType = BLACK_ROOK;   break;
        case 'N': pieceType = WHITE_KNIGHT; break;
        case 'n': pieceType = BLACK_KNIGHT; break;
        case 'B': pieceType = WHITE_BISHOP; break;
        case 'b': pieceType = BLACK_BISHOP; break;
        case 'Q': pieceType = WHITE_QUEEN;  break;
        case 'q': pieceType = BLACK_QUEEN;  break;
        case 'K': pieceType = WHITE_KING;   break;
        case 'k': pieceType = BLACK_KING;   break;
        case 'P': pieceType = WHITE_PAWN;   break;
        case 'p': pieceType = BLACK_PAWN;   break;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
                  pieceType = EMPTY;        break;
        case '/': pieceType = -1;           break;
        default:  pieceType = -1;           break;
        }

        if (pieceType == EMPTY)
        {
            for (j = i + *fen - '0'; i < j; i++)
            {
                pos->pieceType[i] = pieceType;
            }
        } else if (pieceType != -1)
        {
            piece = pieceType  % PIECE_N;
            color = pieceType >= PIECE_N;
            
            pos->color[color] |= sq_bb(i);
            pos->piece[piece] |= sq_bb(i);
            pos->pieceType[i++] = pieceType;
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
