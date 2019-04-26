#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

#define SCREEN_W 400
#define SCREEN_H 400

#define BLACK_PAWN_PATH_DEFAULT "../assets/chess/black_pawn.png"
#define BLACK_ROOK_PATH_DEFAULT "../assets/chess/black_rook.png"
#define BLACK_KNIGHT_PATH_DEFAULT "../assets/chess/black_knight.png"
#define BLACK_BISHOP_PATH_DEFAULT "../assets/chess/black_bishop.png"
#define BLACK_QUEEN_PATH_DEFAULT "../assets/chess/black_queen.png"
#define BLACK_KING_PATH_DEFAULT "../assets/chess/black_king.png"
#define WHITE_PAWN_PATH_DEFAULT "../assets/chess/white_pawn.png"
#define WHITE_ROOK_PATH_DEFAULT "../assets/chess/white_rook.png"
#define WHITE_KNIGHT_PATH_DEFAULT "../assets/chess/white_knight.png"
#define WHITE_BISHOP_PATH_DEFAULT "../assets/chess/white_bishop.png"
#define WHITE_QUEEN_PATH_DEFAULT "../assets/chess/white_queen.png"
#define WHITE_KING_PATH_DEFAULT "../assets/chess/white_king.png"

#define BLACK_PAWN_PATH_GREEN "../assets/chess_green/black_pawn.png"
#define BLACK_ROOK_PATH_GREEN "../assets/chess_green/black_rook.png"
#define BLACK_KNIGHT_PATH_GREEN "../assets/chess_green/black_knight.png"
#define BLACK_BISHOP_PATH_GREEN "../assets/chess_green/black_bishop.png"
#define BLACK_QUEEN_PATH_GREEN "../assets/chess_green/black_queen.png"
#define BLACK_KING_PATH_GREEN "../assets/chess_green/black_king.png"
#define WHITE_PAWN_PATH_GREEN "../assets/chess_green/white_pawn.png"
#define WHITE_ROOK_PATH_GREEN "../assets/chess_green/white_rook.png"
#define WHITE_KNIGHT_PATH_GREEN "../assets/chess_green/white_knight.png"
#define WHITE_BISHOP_PATH_GREEN "../assets/chess_green/white_bishop.png"
#define WHITE_QUEEN_PATH_GREEN "../assets/chess_green/white_queen.png"
#define WHITE_KING_PATH_GREEN "../assets/chess_green/white_king.png"

#define BLACK_PAWN_PATH_PINK "../assets/chess_pink/black_pawn.png"
#define BLACK_ROOK_PATH_PINK "../assets/chess_pink/black_rook.png"
#define BLACK_KNIGHT_PATH_PINK "../assets/chess_pink/black_knight.png"
#define BLACK_BISHOP_PATH_PINK "../assets/chess_pink/black_bishop.png"
#define BLACK_QUEEN_PATH_PINK "../assets/chess_pink/black_queen.png"
#define BLACK_KING_PATH_PINK "../assets/chess_pink/black_king.png"
#define WHITE_PAWN_PATH_PINK "../assets/chess_pink/white_pawn.png"
#define WHITE_ROOK_PATH_PINK "../assets/chess_pink/white_rook.png"
#define WHITE_KNIGHT_PATH_PINK "../assets/chess_pink/white_knight.png"
#define WHITE_BISHOP_PATH_PINK "../assets/chess_pink/white_bishop.png"
#define WHITE_QUEEN_PATH_PINK "../assets/chess_pink/white_queen.png"
#define WHITE_KING_PATH_PINK "../assets/chess_pink/white_king.png"

#define BOARD_PATH_DEFAULT "../assets/chess/board.png"
#define BOARD_PATH_GREEN "../assets/chess_green/board.png"
#define BOARD_PATH_PINK "../assets/chess_pink/board.png"

typedef enum Asset_Color_Enum {
    DEFAULT,
    GREEN,
    PINK
} Asset_Color;

typedef enum Piece_Enum {
    EMPTY,
    BLACK_PAWN,
    BLACK_ROOK,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_QUEEN,
    BLACK_KING,
    WHITE_PAWN,
    WHITE_ROOK,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_QUEEN,
    WHITE_KING,
} Piece_Type;

typedef enum Piece_Color_Enum {
    WHITE,
    BLACK,
    NONE
} Piece_Color;

typedef enum King_State_Enum {
    OK,
    CHECK,
    CHECKMATE
} King_State;

typedef struct Square_Struct {
    int x;
    int y;
} Square;

typedef struct Possible_Moves_Struct {
    Square squares[32];
    int count;
} Possible_Moves;

typedef struct Entity_Struct {
    Piece_Type type;
    Piece_Color color;
    Possible_Moves *possible_moves;
} Entity;

typedef struct Tile_Struct {
    Piece_Type type;
    Piece_Color color;
    char *texture_path_default;
    char *texture_path_green;
    char *texture_path_pink;
    SDL_Texture *texture;
    Possible_Moves *possible_moves;
} Tile;

typedef struct Mouse_State_Struct {
    int x;
    int y;
    int pressed;
} Mouse_State;

typedef struct Selection_Info_Struct {
    int x;
    int y;
    int selected;
    Tile *tile;
} Selection_Info;

typedef struct Game_State_Struct {
    Selection_Info *selection;
    Selection_Info *hovered;
    int player_turn;
    King_State black_king_state;
    King_State white_king_state;
} Game_State;

typedef enum Direction_Enum {
    N,
    E,
    S,
    W,
    NE,
    NW,
    SE,
    SW,
} Direction;

// This is a simple move that only, given a target point, can move to or kill a piece in that square.
void add_simple_possible_move(Possible_Moves *possible_moves, Selection_Info *selection, Tile board[][8], 
                              int up, int left, int right, int down)
{
    // Check bounds.
    if ((selection->y - up >= 0) && (selection->y + down < 8) && 
        (selection->x + right < 8) && (selection->x - left >= 0))
    {
        if ((board[selection->y-up+down][selection->x-left+right].type == EMPTY) || 
            (board[selection->y-up+down][selection->x-left+right].color != selection->tile->color))
        {
            possible_moves->squares[possible_moves->count].x = selection->x-left+right;
            possible_moves->squares[possible_moves->count].y = selection->y-up+down;
            possible_moves->count++;
        }
    }
}

// Long move in a direction that stops at first collision, but can kill enemies.
void add_possible_moves_long(Possible_Moves *possible_moves, Selection_Info *selection, Tile board[][8], Direction direction)
{
    int x_increment = 0;
    int y_increment = 0;

    if (direction == N || direction == NE || direction == NW)
    {
        x_increment = -1;
    }

    if (direction == S || direction == SE || direction == SW)
    {
        x_increment = 1;
    }

    if (direction == W || direction == NW || direction == SW)
    {
        y_increment = -1;
    }

    if (direction == E || direction == NE || direction == SE)
    {
        y_increment = 1;
    }

    int x = selection->x;
    int y = selection->y;

    // TODO(bkaylor): Cleanup this mess...
    while (1)
    {
        x += x_increment;
        y += y_increment;

        if ((0 <= x && x < 8) && (0 <= y && y < 8))
        {
            if (board[y][x].type == EMPTY)
            {
                possible_moves->squares[possible_moves->count].x = x;
                possible_moves->squares[possible_moves->count].y = y;
                possible_moves->count++;
            }
            else if (board[y][x].color != selection->tile->color)
            {
                possible_moves->squares[possible_moves->count].x = x;
                possible_moves->squares[possible_moves->count].y = y;
                possible_moves->count++;

                break;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

void get_possible_moves(Possible_Moves *possible_moves, Selection_Info *selection, Tile board[][8])
{
    possible_moves->count = 0;

    switch (selection->tile->type)
    {
        case EMPTY:
        break;

        case WHITE_PAWN:
            // One space forward.
            if (((selection->y - 1) >= 0) && (board[selection->y-1][selection->x].type == EMPTY))
            {
                possible_moves->squares[possible_moves->count].x = selection->x;
                possible_moves->squares[possible_moves->count].y = selection->y - 1;
                possible_moves->count++;
            }

            // Two spaces forward.
            if ((selection->y == 6) && 
                (board[selection->y-1][selection->x].type == EMPTY) && 
                (board[selection->y-2][selection->x].type == EMPTY))
            {
                possible_moves->squares[possible_moves->count].x = selection->x;
                possible_moves->squares[possible_moves->count].y = selection->y - 2;
                possible_moves->count++;
            }

            // Diagonal to the left (attack).
            if (((selection->x - 1) >= 0) && ((selection->y - 1) >= 0) && 
                (board[selection->y - 1][selection->x - 1].color == BLACK))
            {
                possible_moves->squares[possible_moves->count].x = selection->x-1;
                possible_moves->squares[possible_moves->count].y = selection->y-1;
                possible_moves->count++;
            }

            // Diagonal to the right (attack).
            if (((selection->x + 1) < 8) && ((selection->y - 1) >= 0) && 
                (board[selection->y - 1][selection->x + 1].color == BLACK))
            {
                possible_moves->squares[possible_moves->count].x = selection->x+1;
                possible_moves->squares[possible_moves->count].y = selection->y-1;
                possible_moves->count++;
            }
        break;

        case BLACK_PAWN:
            // One space forward.
            if (((selection->y + 1) < 8) && (board[selection->y+1][selection->x].type == EMPTY))
            {
                possible_moves->squares[possible_moves->count].x = selection->x;
                possible_moves->squares[possible_moves->count].y = selection->y+1;
                possible_moves->count++;
            }

            // Two spaces forward.
            if ((selection->y == 1) && 
                (board[selection->y+1][selection->x].type == EMPTY) && 
                (board[selection->y+2][selection->x].type == EMPTY))
            {
                possible_moves->squares[possible_moves->count].x = selection->x;
                possible_moves->squares[possible_moves->count].y = selection->y+2;
                possible_moves->count++;
            }

            // Diagonal to the left (attack).
            if (((selection->x + 1) < 8) && ((selection->y + 1) < 8) && 
                (board[selection->y + 1][selection->x + 1].color == WHITE))
            {
                possible_moves->squares[possible_moves->count].x = selection->x+1;
                possible_moves->squares[possible_moves->count].y = selection->y+1;
                possible_moves->count++;
            }

            // Diagonal to the right (attack).
            if (((selection->x - 1) >= 0) && ((selection->y + 1) < 8) && 
                (board[selection->y + 1][selection->x - 1].color == WHITE))
            {
                possible_moves->squares[possible_moves->count].x = selection->x-1;
                possible_moves->squares[possible_moves->count].y = selection->y+1;
                possible_moves->count++;
            }
        break;

        case WHITE_ROOK:
        case BLACK_ROOK:
            add_possible_moves_long(possible_moves, selection, board, N);
            add_possible_moves_long(possible_moves, selection, board, E);
            add_possible_moves_long(possible_moves, selection, board, S);
            add_possible_moves_long(possible_moves, selection, board, W);
        break;

        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            add_simple_possible_move(possible_moves, selection, board, 2, 1, 0, 0);
            add_simple_possible_move(possible_moves, selection, board, 2, 0, 1, 0);

            add_simple_possible_move(possible_moves, selection, board, 1, 2, 0, 0);
            add_simple_possible_move(possible_moves, selection, board, 0, 2, 0, 1);

            add_simple_possible_move(possible_moves, selection, board, 1, 0, 2, 0);
            add_simple_possible_move(possible_moves, selection, board, 0, 0, 2, 1);

            add_simple_possible_move(possible_moves, selection, board, 0, 1, 0, 2);
            add_simple_possible_move(possible_moves, selection, board, 0, 0, 1, 2);
        break;

        case WHITE_BISHOP:
        case BLACK_BISHOP:
            add_possible_moves_long(possible_moves, selection, board, NE);
            add_possible_moves_long(possible_moves, selection, board, NW);
            add_possible_moves_long(possible_moves, selection, board, SE);
            add_possible_moves_long(possible_moves, selection, board, SW);
        break;

        case WHITE_QUEEN:
        case BLACK_QUEEN:
            add_possible_moves_long(possible_moves, selection, board, N);
            add_possible_moves_long(possible_moves, selection, board, E);
            add_possible_moves_long(possible_moves, selection, board, S);
            add_possible_moves_long(possible_moves, selection, board, W);

            add_possible_moves_long(possible_moves, selection, board, NE);
            add_possible_moves_long(possible_moves, selection, board, NW);
            add_possible_moves_long(possible_moves, selection, board, SE);
            add_possible_moves_long(possible_moves, selection, board, SW);
        break;

        case WHITE_KING:
        case BLACK_KING:
            add_simple_possible_move(possible_moves, selection, board, 1, 0, 0, 0);
            add_simple_possible_move(possible_moves, selection, board, 0, 1, 0, 0);
            add_simple_possible_move(possible_moves, selection, board, 0, 0, 1, 0);
            add_simple_possible_move(possible_moves, selection, board, 0, 0, 0, 1);

            add_simple_possible_move(possible_moves, selection, board, 1, 1, 0, 0);
            add_simple_possible_move(possible_moves, selection, board, 1, 0, 1, 0);
            add_simple_possible_move(possible_moves, selection, board, 0, 1, 0, 1);
            add_simple_possible_move(possible_moves, selection, board, 0, 0, 1, 1);
        break;

        default:
        break;

    }
}

int is_possible_move(Possible_Moves *possible_moves, int x, int y)
{
    for (int i = 0; i < possible_moves->count; i++)
    {
        if ((possible_moves->squares[i].x == x) && (possible_moves->squares[i].y == y))
        {
            return 1;
        }
    }

    return 0;
}

Tile starting_board[8][8] = {
    {
        {BLACK_ROOK, BLACK, BLACK_ROOK_PATH_DEFAULT, BLACK_ROOK_PATH_GREEN, BLACK_ROOK_PATH_PINK, NULL}, 
        {BLACK_BISHOP, BLACK, BLACK_BISHOP_PATH_DEFAULT, BLACK_BISHOP_PATH_GREEN, BLACK_BISHOP_PATH_PINK, NULL}, 
        {BLACK_KNIGHT, BLACK, BLACK_KNIGHT_PATH_DEFAULT, BLACK_KNIGHT_PATH_GREEN, BLACK_KNIGHT_PATH_PINK, NULL},
        {BLACK_QUEEN, BLACK, BLACK_QUEEN_PATH_DEFAULT, BLACK_QUEEN_PATH_GREEN, BLACK_QUEEN_PATH_PINK, NULL},
        {BLACK_KING, BLACK, BLACK_KING_PATH_DEFAULT, BLACK_KING_PATH_GREEN, BLACK_KING_PATH_PINK, NULL},
        {BLACK_KNIGHT, BLACK, BLACK_KNIGHT_PATH_DEFAULT, BLACK_KNIGHT_PATH_GREEN, BLACK_KNIGHT_PATH_PINK, NULL},
        {BLACK_BISHOP, BLACK, BLACK_BISHOP_PATH_DEFAULT, BLACK_BISHOP_PATH_GREEN, BLACK_BISHOP_PATH_PINK, NULL},
        {BLACK_ROOK, BLACK, BLACK_ROOK_PATH_DEFAULT, BLACK_ROOK_PATH_GREEN, BLACK_ROOK_PATH_PINK, NULL}
    },
    {
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH_DEFAULT, BLACK_PAWN_PATH_GREEN, BLACK_PAWN_PATH_PINK, NULL}, 
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH_DEFAULT, BLACK_PAWN_PATH_GREEN, BLACK_PAWN_PATH_PINK, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH_DEFAULT, BLACK_PAWN_PATH_GREEN, BLACK_PAWN_PATH_PINK, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH_DEFAULT, BLACK_PAWN_PATH_GREEN, BLACK_PAWN_PATH_PINK, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH_DEFAULT, BLACK_PAWN_PATH_GREEN, BLACK_PAWN_PATH_PINK, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH_DEFAULT, BLACK_PAWN_PATH_GREEN, BLACK_PAWN_PATH_PINK, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH_DEFAULT, BLACK_PAWN_PATH_GREEN, BLACK_PAWN_PATH_PINK, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH_DEFAULT, BLACK_PAWN_PATH_GREEN, BLACK_PAWN_PATH_PINK, NULL}
    },
    {
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},    
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},    
    },
    {
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},    
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},    
    },
    {
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},    
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},    
    },
    {
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},    
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},
        {EMPTY, NONE, "", NULL},    
    },
    {
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH_DEFAULT, WHITE_PAWN_PATH_GREEN, WHITE_PAWN_PATH_PINK, NULL}, 
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH_DEFAULT, WHITE_PAWN_PATH_GREEN, WHITE_PAWN_PATH_PINK, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH_DEFAULT, WHITE_PAWN_PATH_GREEN, WHITE_PAWN_PATH_PINK, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH_DEFAULT, WHITE_PAWN_PATH_GREEN, WHITE_PAWN_PATH_PINK, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH_DEFAULT, WHITE_PAWN_PATH_GREEN, WHITE_PAWN_PATH_PINK, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH_DEFAULT, WHITE_PAWN_PATH_GREEN, WHITE_PAWN_PATH_PINK, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH_DEFAULT, WHITE_PAWN_PATH_GREEN, WHITE_PAWN_PATH_PINK, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH_DEFAULT, WHITE_PAWN_PATH_GREEN, WHITE_PAWN_PATH_PINK, NULL}
    },
    {
        {WHITE_ROOK, WHITE, WHITE_ROOK_PATH_DEFAULT, WHITE_ROOK_PATH_GREEN, WHITE_ROOK_PATH_PINK, NULL}, 
        {WHITE_BISHOP, WHITE, WHITE_BISHOP_PATH_DEFAULT, WHITE_BISHOP_PATH_GREEN, WHITE_BISHOP_PATH_PINK, NULL}, 
        {WHITE_KNIGHT, WHITE, WHITE_KNIGHT_PATH_DEFAULT, WHITE_KNIGHT_PATH_GREEN, WHITE_KNIGHT_PATH_PINK, NULL},
        {WHITE_KING, WHITE, WHITE_KING_PATH_DEFAULT, WHITE_KING_PATH_GREEN, WHITE_KING_PATH_PINK, NULL},
        {WHITE_QUEEN, WHITE, WHITE_QUEEN_PATH_DEFAULT, WHITE_QUEEN_PATH_GREEN, WHITE_QUEEN_PATH_PINK, NULL},
        {WHITE_KNIGHT, WHITE, WHITE_KNIGHT_PATH_DEFAULT, WHITE_KNIGHT_PATH_GREEN, WHITE_KNIGHT_PATH_PINK, NULL},
        {WHITE_BISHOP, WHITE, WHITE_BISHOP_PATH_DEFAULT, WHITE_BISHOP_PATH_GREEN, WHITE_BISHOP_PATH_PINK, NULL},
        {WHITE_ROOK, WHITE, WHITE_ROOK_PATH_DEFAULT, WHITE_ROOK_PATH_GREEN, WHITE_ROOK_PATH_PINK, NULL}
    },
};

SDL_Texture *board_texture;
SDL_Texture *dog_texture;

// TODO(bkaylor): These circles look bad ...
void draw_circle(SDL_Renderer *ren, SDL_Point center, int radius, SDL_Color color)
{
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(ren, center.x + dx, center.y + dy);
            }
        }
    }
}

void render(SDL_Renderer *renderer, Tile board[][8], Game_State *state, int tile_side_length, int side_buffer_length, Asset_Color color)
{
    float percent_of_tile_for_piece_to_take_up = 0.85f;
    float percent_of_tile_for_move_circle_to_take_up = 0.18f;
    SDL_RenderClear(renderer);

    // Set background color.
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderFillRect(renderer, NULL);

    // Draw checkered background.
    SDL_Rect background_rect;
    background_rect.x = side_buffer_length - (tile_side_length * 0.08);
    background_rect.y = side_buffer_length - (tile_side_length * 0.08);
    background_rect.w = tile_side_length * 8 + (tile_side_length * 0.20);
    background_rect.h = tile_side_length * 8 + (tile_side_length * 0.20);
    SDL_RenderCopy(renderer, board_texture, NULL, &background_rect);

    // Color kings in check.
    SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255);
    if (state->black_king_state == CHECK)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (board[i][j].type == BLACK_KING)
                {
                    SDL_Rect black_king_rect;
                    black_king_rect.x = (j * tile_side_length) + side_buffer_length;
                    black_king_rect.y = (i * tile_side_length) + side_buffer_length;
                    black_king_rect.w = tile_side_length;
                    black_king_rect.h = tile_side_length;

                    SDL_RenderFillRect(renderer, &black_king_rect);
                }
            }
        }
    }

    if (state->white_king_state == CHECK)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (board[i][j].type == WHITE_KING)
                {
                    SDL_Rect white_king_rect;
                    white_king_rect.x = (j * tile_side_length) + side_buffer_length;
                    white_king_rect.y = (i * tile_side_length) + side_buffer_length;
                    white_king_rect.w = tile_side_length;
                    white_king_rect.h = tile_side_length;

                    SDL_RenderFillRect(renderer, &white_king_rect);
                }
            }
        }
    }

    // Color selection.
    if (state->selection->selected)
    {
        SDL_Rect selection_rect;
        selection_rect.x = (state->selection->x * tile_side_length) + side_buffer_length;
        selection_rect.y = (state->selection->y * tile_side_length) + side_buffer_length;
        selection_rect.w = tile_side_length;
        selection_rect.h = tile_side_length;

        SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
        SDL_RenderFillRect(renderer, &selection_rect);

    }

#if 0
    // Color hovered.
    SDL_Rect hovered_rect;
    hovered_rect.x = (state->hovered->x * tile_side_length) + side_buffer_length;
    hovered_rect.y = (state->hovered->y * tile_side_length) + side_buffer_length;
    hovered_rect.w = tile_side_length;
    hovered_rect.h = tile_side_length;

    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    SDL_RenderFillRect(renderer, &hovered_rect);
#endif

    SDL_Rect tile_rect;
    tile_rect.x = side_buffer_length + (tile_side_length * ((1.0f - percent_of_tile_for_piece_to_take_up) / 2));
    tile_rect.y = side_buffer_length + (tile_side_length * ((1.0f - percent_of_tile_for_piece_to_take_up) / 2));
    tile_rect.w = tile_side_length * percent_of_tile_for_piece_to_take_up;
    tile_rect.h = tile_side_length * percent_of_tile_for_piece_to_take_up;

    // Draw board.
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].type != EMPTY)
            {
                SDL_RenderCopy(renderer, board[i][j].texture, NULL, &tile_rect);
            }

            tile_rect.x += tile_side_length;
        }
        tile_rect.x = side_buffer_length + (tile_side_length * ((1.0f - percent_of_tile_for_piece_to_take_up) / 2));
        tile_rect.y += tile_side_length;
    }

    // Color available moves. 
    if (state->selection->selected)
    {
        SDL_Point move_point = {0};

        Possible_Moves *possible_moves = state->selection->tile->possible_moves; 

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Color color_move = {0, 153, 76, 255};
        SDL_Color color_kill = {153, 0, 0, 255};

        for (int i = 0; i < possible_moves->count; i++)
        {
            move_point.x = ((possible_moves->squares[i].x) * tile_side_length) + side_buffer_length + (tile_side_length / 2);
            move_point.y = ((possible_moves->squares[i].y) * tile_side_length) + side_buffer_length + (tile_side_length / 2);

            if (board[possible_moves->squares[i].y][possible_moves->squares[i].x].type != EMPTY)
            {
                draw_circle(renderer, move_point, (int) (percent_of_tile_for_move_circle_to_take_up * tile_side_length), color_kill);
            }
            else
            {
                draw_circle(renderer, move_point, (int) (percent_of_tile_for_move_circle_to_take_up * tile_side_length), color_move);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void load_images(SDL_Renderer *renderer, Asset_Color color) 
{
    SDL_Surface *surface;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (starting_board[i][j].type != EMPTY)
            {
                if (color == DEFAULT)
                {
                    surface = IMG_Load(starting_board[i][j].texture_path_default);
                }
                else if (color == GREEN)
                {
                    surface = IMG_Load(starting_board[i][j].texture_path_green);
                }
                else
                {
                    surface = IMG_Load(starting_board[i][j].texture_path_pink);
                }
                starting_board[i][j].texture = SDL_CreateTextureFromSurface(renderer, surface);
            }
        }
    }
    SDL_FreeSurface(surface);

    SDL_Surface *board_surface;
    if (color == DEFAULT)
    {
        board_surface = IMG_Load(BOARD_PATH_DEFAULT);
    }
    else if (color == GREEN)
    {
        board_surface = IMG_Load(BOARD_PATH_GREEN);
    }
    else
    {
        board_surface = IMG_Load(BOARD_PATH_PINK);
    }
    board_texture = SDL_CreateTextureFromSurface(renderer, board_surface);
    SDL_FreeSurface(board_surface);
}

int update(Tile board[][8], Game_State *state, Mouse_State *mouse_state, int tile_side_length, int side_buffer_length)
{
    state->hovered->x = (mouse_state->x -side_buffer_length) / tile_side_length;
    state->hovered->y = (mouse_state->y -side_buffer_length) / tile_side_length;
    state->hovered->tile = &board[state->hovered->y][state->hovered->x];

    if (state->hovered->x > 7) state->hovered->x = 7;
    if (state->hovered->y > 7) state->hovered->y = 7;

    // Get every piece's possible moves.
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[j][i].type != EMPTY)
            {
                Selection_Info temp_selection = {0};
                temp_selection.x = i;
                temp_selection.y = j;
                temp_selection.selected = 0;
                temp_selection.tile = &board[j][i];

                get_possible_moves(temp_selection.tile->possible_moves, &temp_selection, board);
            }
        }
    }

    // Add data about threatened squares.
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[j][i].type == BLACK_KING || board[j][i].type == WHITE_KING)
            {
            }
        }
    }


    if (state->player_turn)
    {
        if (state->selection->selected) 
        {
            Possible_Moves *possible_moves = state->selection->tile->possible_moves; 

            if (mouse_state->pressed == SDL_BUTTON_LEFT)
            {
                if (is_possible_move(possible_moves, state->hovered->x, state->hovered->y))
                {
                    board[state->hovered->y][state->hovered->x].type = state->selection->tile->type;
                    board[state->hovered->y][state->hovered->x].color= state->selection->tile->color;
                    board[state->hovered->y][state->hovered->x].texture = state->selection->tile->texture;
                    board[state->hovered->y][state->hovered->x].texture_path_default = state->selection->tile->texture_path_default;
                    board[state->hovered->y][state->hovered->x].texture_path_green = state->selection->tile->texture_path_green;
                    board[state->hovered->y][state->hovered->x].texture_path_pink = state->selection->tile->texture_path_pink;

                    // Reset the square the piece moved from.
                    board[state->selection->y][state->selection->x].type = EMPTY;
                    board[state->selection->y][state->selection->x].color = NONE;
                    state->selection->selected = 0;

                    state->player_turn = 0;
                }
            }
            else if (mouse_state->pressed == SDL_BUTTON_RIGHT)
            {
                state->selection->selected = 0;
            }
        }
        else
        {
            // TODO(bkaylor): This is not the right thing to do! King in check doesn't mean king must move.
            // Other pieces can make legal moves when the king is in check.
            // What should happen, is there should be a function to check the state of the king given a move is made.
            if ((mouse_state->pressed == SDL_BUTTON_LEFT) && (state->hovered->tile->type != EMPTY) && 
                ((state->white_king_state != CHECK) || (state->hovered->tile->type == WHITE_KING)))
            {
                state->selection->x = state->hovered->x;
                state->selection->y = state->hovered->y;
                state->selection->tile = state->hovered->tile;
                state->selection->selected = 1;
            }
        }
    }
    else
    {
        // AI's turn.

        // Get a random piece's possible moves.
        Tile black_pieces[16]; 
        Square black_pieces_locations[16];
        int black_pieces_count = 0;

        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (board[i][j].color == BLACK)
                {
                    black_pieces[black_pieces_count].type = board[i][j].type;
                    black_pieces[black_pieces_count].color = board[i][j].color;
                    black_pieces[black_pieces_count].texture = board[i][j].texture;
                    black_pieces[black_pieces_count].possible_moves = board[i][j].possible_moves;

                    black_pieces_locations[black_pieces_count].x = j;
                    black_pieces_locations[black_pieces_count].y = i;

                    black_pieces_count++;
                }
            }
        }

        Selection_Info selection = {0};

        do
        {
            int random_piece_index = rand() % black_pieces_count;
            selection.x = black_pieces_locations[random_piece_index].x;
            selection.y = black_pieces_locations[random_piece_index].y;
            selection.tile = &black_pieces[random_piece_index];
        } while (selection.tile->possible_moves->count == 0);

        Possible_Moves *possible_moves = selection.tile->possible_moves;

        // Get a random possible move.
        int random_move = rand() % possible_moves->count;
        board[possible_moves->squares[random_move].y][possible_moves->squares[random_move].x].type = selection.tile->type;
        board[possible_moves->squares[random_move].y][possible_moves->squares[random_move].x].color = selection.tile->color;
        board[possible_moves->squares[random_move].y][possible_moves->squares[random_move].x].texture = selection.tile->texture;
        board[possible_moves->squares[random_move].y][possible_moves->squares[random_move].x].texture_path_default = selection.tile->texture_path_default;
        board[possible_moves->squares[random_move].y][possible_moves->squares[random_move].x].texture_path_green = selection.tile->texture_path_green;
        board[possible_moves->squares[random_move].y][possible_moves->squares[random_move].x].texture_path_pink = selection.tile->texture_path_pink;

        // Reset the square the piece moved from.
        board[selection.y][selection.x].type = EMPTY;
        board[selection.y][selection.x].color = NONE;

        state->player_turn = 1;
    }

    // Figure out the kings' state.
    state->black_king_state = OK;
    state->white_king_state = OK;

    // White king state.
    Tile black_pieces[16]; 
    Square black_pieces_locations[16];
    int black_pieces_count = 0;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].color == BLACK)
            {
                black_pieces[black_pieces_count].type = board[i][j].type;
                black_pieces[black_pieces_count].color = board[i][j].color;
                black_pieces[black_pieces_count].texture = board[i][j].texture;
                black_pieces[black_pieces_count].possible_moves = board[i][j].possible_moves;

                black_pieces_locations[black_pieces_count].x = i;
                black_pieces_locations[black_pieces_count].y = j;

                black_pieces_count++;

            }
        }
    }

    for (int i = 0; i < black_pieces_count; i++)
    {
        for (int n = 0; n < black_pieces[i].possible_moves->count; n++)
        {
            int x = black_pieces[i].possible_moves->squares[n].x;
            int y = black_pieces[i].possible_moves->squares[n].y;

            if (board[y][x].type == WHITE_KING)
            {
                state->white_king_state = CHECK;
            }
        }
    }

    // Black king state.
    Tile white_pieces[16]; 
    Square white_pieces_locations[16];
    int white_pieces_count = 0;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].color == WHITE)
            {
                white_pieces[white_pieces_count].type = board[i][j].type;
                white_pieces[white_pieces_count].color = board[i][j].color;
                white_pieces[white_pieces_count].texture = board[i][j].texture;
                white_pieces[white_pieces_count].possible_moves = board[i][j].possible_moves;

                white_pieces_locations[white_pieces_count].x = i;
                white_pieces_locations[white_pieces_count].y = j;

                white_pieces_count++;
            }
        }
    }

    for (int i = 0; i < white_pieces_count; i++)
    {
        for (int n = 0; n < white_pieces[i].possible_moves->count; n++)
        {
            int x = white_pieces[i].possible_moves->squares[n].x;
            int y = white_pieces[i].possible_moves->squares[n].y;

            if (board[y][x].type == BLACK_KING)
            {
                state->black_king_state = CHECK;
            }
        }
    }

    // Check if someone has won.
    int kings = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].type == BLACK_KING || board[i][j].type == WHITE_KING)
            {
                kings++;
            }
        }
    }

    if (kings != 2)
    {
        return 1;
    }

    return 0;
}

void get_input(int *quit, Mouse_State *mouse_state, Asset_Color *color, SDL_Renderer *ren)
{
    // Get mouse info.
    mouse_state->pressed = 0;
    SDL_GetMouseState(&mouse_state->x, &mouse_state->y);

    // Handle events.
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        *quit = 1;
                        break;

                    case SDLK_c:
                        if (*color == DEFAULT) {
                            *color = GREEN;
                        } else if (*color == GREEN) {
                            *color = PINK;
                        } else if (*color == PINK) {
                            *color = DEFAULT;
                        }

                        load_images(ren, *color);
                        break;

                    default:
                        break;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                mouse_state->pressed = event.button.button;
                break;

            case SDL_QUIT:
                *quit = 1;
                break;

            default:
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    printf("Chess is a game...\n");
	SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init video error: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        printf("SDL_Init audio error: %s\n", SDL_GetError());
        return 1;
    }

    // SDL_ShowCursor(SDL_DISABLE);

	// Setup window
	SDL_Window *win = SDL_CreateWindow("Chess",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			SCREEN_W, SCREEN_H,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	// Setup renderer
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// Setup font
	TTF_Init();
	TTF_Font *font = TTF_OpenFont("liberation.ttf", 12);
	if (!font)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error: Font", TTF_GetError(), win);
		return -666;
	}

    Asset_Color color = DEFAULT;

    load_images(ren, color);

    // Setup main loop
    srand(time(NULL));
    int quit = 0;
    Mouse_State mouse_state = {0};

    /*
    Tile board[8][8];
    memcpy(board, starting_board, sizeof(starting_board[0]) * 8 * 8);
    */

    // TODO(bkaylor): Can I get away with not malloc'ing this?
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            starting_board[j][i].possible_moves = malloc(sizeof(Possible_Moves));
            // starting_board[j][i].possible_moves->squares[32];
            starting_board[j][i].possible_moves->count = 0;
        }
    }

    Selection_Info hovered = {0};
    Selection_Info selection = {0};
    Game_State state = {0};
    state.hovered = &hovered;
    state.selection = &selection;
    state.player_turn = 1;
    state.black_king_state = OK;
    state.white_king_state = OK;

    int window_w, window_h;

    // Main loop
    const float FPS_INTERVAL = 1.0f;
    Uint64 fps_start, fps_current, fps_frames = 0;

    fps_start = SDL_GetTicks();

    while (!quit)
    {
        SDL_PumpEvents();
        get_input(&quit, &mouse_state, &color, ren);

        if (!quit)
        {
            SDL_GetWindowSize(win, &window_w, &window_h);

            int tile_side_length = window_w < window_h ? window_w / 10 : window_h / 10;
            int side_buffer_length = window_w < window_h ? window_w / 20 : window_h / 20;

            quit = update(starting_board, &state, &mouse_state, tile_side_length, side_buffer_length);
            render(ren, starting_board, &state, tile_side_length, side_buffer_length, color);

            fps_frames++;

            if (fps_start < SDL_GetTicks() - FPS_INTERVAL * 1000)
            {
                fps_start = SDL_GetTicks();
                fps_current = fps_frames;
                fps_frames = 0;

                printf("%I64d fps\n", fps_current);
            }
        }
    }

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
    return 0;
}
