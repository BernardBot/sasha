#include <stdint.h>
#include <stdio.h>
#include "util.h"
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
        const int toPiece = toType % PIECE_N;
        const uint64_t toSquare = sq_bb(to);

        pos->piece[toPiece]    ^= toSquare;
        pos->color[!fromColor] ^= toSquare;
        // pos->pieceType[to]   = EMPTY; already overwritten by moving the piece

        pos->state->capturedSquare    = to;
        pos->state->capturedPieceType = toType;
    } else if (tag == CASTLING)
    {
        // we have to move the rook (king already done)
        const int rookType = fromColor * PIECE_N + ROOK;
        const int rookFrom = to > from ? to + 1 : to - 2;
        const int rookTo   = to > from ? to - 1 : to + 1;
        const uint64_t rookFromToSquare = sq_bb(rookFrom) | sq_bb(rookTo);

        pos->piece[ROOK]        ^= rookFromToSquare;
        pos->color[fromColor]   ^= rookFromToSquare;
        pos->pieceType[rookFrom] = EMPTY;
        pos->pieceType[rookTo]   = rookType;
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
        } else if (to - from == 16 || to - from == -16) // update enpassant state
        {
            pos->state->enpassant = fromColor == WHITE ? to + 8 : to - 8;
        }
    } else if (fromPiece == KING) // update castling state
    {
        pos->state->castling &= ~(OO[fromColor] | OOO[fromColor]);
    } else if (fromPiece == ROOK)
    {
        if (from == BRANK[fromColor][A8])
        {
            pos->state->castling &= ~(OOO[fromColor]);
        } else if (from == BRANK[fromColor][H8])
        {
            pos->state->castling &= ~(OO[fromColor]);
        }
    }
}

void undoMove(uint16_t move, struct Position *pos)
{
    // parse move
    const int from =  move        & 0b111111;
    const int to   = (move >> 6)  & 0b111111;
    const int tag  = (move >> 12) & 0b11;

    // assume there is a piece on the to index square
    const int toType = pos->pieceType[to];
    // const int fromType  = pos->pieceType[from]; should be empty

    // convert pieceType to piece and color
    const int toPiece = toType  % PIECE_N;
    const int toColor = toType >= PIECE_N;

    const uint64_t fromToSquare = sq_bb(from) | sq_bb(to);

    // unmove piece
    pos->piece[toPiece] ^= fromToSquare;
    pos->color[toColor] ^= fromToSquare;
    pos->pieceType[from] = toType;
    pos->pieceType[to]   = EMPTY;

    if (pos->state->capturedSquare != -1)
    {
        const int capPiece = pos->state->capturedPieceType  % PIECE_N;
        const int capColor = pos->state->capturedPieceType >= PIECE_N;
        const uint64_t capSquare = sq_bb(pos->state->capturedSquare);

        pos->piece[capPiece] ^= capSquare;
        pos->color[capColor] ^= capSquare;
        pos->pieceType[pos->state->capturedSquare] = pos->state->capturedPieceType;

    } else if (tag == CASTLING)
    {
        // move the rook back
        const int rookType = toColor * PIECE_N + ROOK;
        const int rookFrom = to > from ? to + 1 : to - 2;
        const int rookTo   = to > from ? to - 1 : to + 1;
        const uint64_t rookFromToSquare = sq_bb(rookFrom) | sq_bb(rookTo);

        pos->piece[ROOK]        ^= rookFromToSquare;
        pos->color[toColor]     ^= rookFromToSquare;
        pos->pieceType[rookFrom] = rookType;
        pos->pieceType[rookTo]   = EMPTY;

    }

    if (tag == PROMOTION)
    {
        const uint64_t fromSquare = sq_bb(from);
        pos->piece[PAWN]    ^= fromSquare;
        pos->piece[toPiece] ^= fromSquare;
        pos->pieceType[from] = toColor * PIECE_N + PAWN;
    }

    // undo the state
    pos->state = pos->state->previousState;
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
