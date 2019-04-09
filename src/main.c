#include <stdio.h>
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

#define SCREEN_W 180
#define SCREEN_H 180

#define BLACK_PAWN_PATH "../assets/chess/black_pawn.png"
#define BLACK_ROOK_PATH "../assets/chess/black_rook.png"
#define BLACK_KNIGHT_PATH "../assets/chess/black_knight.png"
#define BLACK_BISHOP_PATH "../assets/chess/black_bishop.png"
#define BLACK_QUEEN_PATH "../assets/chess/black_queen.png"
#define BLACK_KING_PATH "../assets/chess/black_king.png"
#define WHITE_PAWN_PATH "../assets/chess/white_pawn.png"
#define WHITE_ROOK_PATH "../assets/chess/white_rook.png"
#define WHITE_KNIGHT_PATH "../assets/chess/white_knight.png"
#define WHITE_BISHOP_PATH "../assets/chess/white_bishop.png"
#define WHITE_QUEEN_PATH "../assets/chess/white_queen.png"
#define WHITE_KING_PATH "../assets/chess/white_king.png"
#define BOARD_PATH "../assets/chess/board.bmp"
#define DOG_PATH "../assets/chess/dog.bmp"

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

typedef struct Tile_Struct {
    Piece_Type type;
    Piece_Color color;
    char *texture_path;
    SDL_Texture *texture;
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
} Game_State;

typedef struct Square_Struct {
    int x;
    int y;
} Square;

typedef struct Possible_Moves_Struct {
    Square squares[30];
    int count;
} Possible_Moves;

typedef enum Move_Killability_Enum {
    CAN,
    CANNOT,
    MUST,
} Move_Killability;

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

Tile starting_board[8][8] = {
    {
        {BLACK_ROOK, BLACK, BLACK_ROOK_PATH, NULL}, 
        {BLACK_BISHOP, BLACK, BLACK_BISHOP_PATH, NULL}, 
        {BLACK_KNIGHT, BLACK, BLACK_KNIGHT_PATH, NULL},
        {BLACK_QUEEN, BLACK, BLACK_QUEEN_PATH, NULL},
        {BLACK_KING, BLACK, BLACK_KING_PATH, NULL},
        {BLACK_KNIGHT, BLACK, BLACK_KNIGHT_PATH, NULL},
        {BLACK_BISHOP, BLACK, BLACK_BISHOP_PATH, NULL},
        {BLACK_ROOK, BLACK, BLACK_ROOK_PATH, NULL}
    },
    {
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH, NULL}, 
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK, BLACK_PAWN_PATH, NULL}
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
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH, NULL}, 
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE, WHITE_PAWN_PATH, NULL}
    },
    {
        {WHITE_ROOK, WHITE, WHITE_ROOK_PATH, NULL}, 
        {WHITE_BISHOP, WHITE, WHITE_BISHOP_PATH, NULL}, 
        {WHITE_KNIGHT, WHITE, WHITE_KNIGHT_PATH, NULL},
        {WHITE_KING, WHITE, WHITE_KING_PATH, NULL},
        {WHITE_QUEEN, WHITE, WHITE_QUEEN_PATH, NULL},
        {WHITE_KNIGHT, WHITE, WHITE_KNIGHT_PATH, NULL},
        {WHITE_BISHOP, WHITE, WHITE_BISHOP_PATH, NULL},
        {WHITE_ROOK, WHITE, WHITE_ROOK_PATH, NULL}
    },
};

SDL_Texture *board_texture;
SDL_Texture *dog_texture;

void render(SDL_Renderer *renderer, Tile board[][8], Game_State *state)
{
    SDL_RenderClear(renderer);

    // Draw background.
    SDL_RenderCopy(renderer, board_texture, NULL, NULL);

    SDL_Rect tile_rect;
    tile_rect.x = 0 + 2;
    tile_rect.y = 0 + 2;
    tile_rect.w = 22;
    tile_rect.h = 22;

    // Color selection.
    if (state->selection->selected)
    {
        SDL_Rect selection_rect;
        selection_rect.x = (state->selection->x * 22) + 2;
        selection_rect.y = (state->selection->y * 22) + 2;
        selection_rect.w = 22;
        selection_rect.h = 22;

        SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
        SDL_RenderFillRect(renderer, &selection_rect);

        // Color available moves. 
        SDL_Rect move_rect;
        move_rect.x = (state->selection->x * 22) + 2;
        move_rect.y = (state->selection->y * 22) + 2;
        move_rect.w = 22;
        move_rect.h = 22;
        
        //
        // Top left corner is (0, 0)
        // Bottom left corner is (0, 7)
        // Top right corner is (7, 0)
        // Button right corner is (7, 7)
        //

        // TODO(bkaylor): This should live in the game state, so we know if they are making a valid move or not.
        //                Also so that the AI can build their own and use it to play.
        Possible_Moves possible_moves; 
        possible_moves.count = 0;

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

        Selection_Info *selection = state->selection;
        // TODO(bkaylor): Do this for black pieces too.
        switch (selection->tile->type)
        {
            case EMPTY:
            break;

            case WHITE_PAWN:
                // One space forward.
                if (((selection->y - 1) < 8) && (board[selection->y-1][selection->x].type == EMPTY))
                {
                    move_rect.x = (selection->x * 22) + 2;
                    move_rect.y = ((selection->y-1) * 22) + 2;

                    SDL_RenderFillRect(renderer, &move_rect);
                }

                // Two spaces forward.
                if ((selection->y == 6) && 
                    (board[selection->y-1][selection->x].type == EMPTY) && 
                    (board[selection->y-2][selection->x].type == EMPTY))
                {
                    move_rect.x = (selection->x * 22) +2;
                    move_rect.y = ((selection->y -2) * 22) + 2;

                    SDL_RenderFillRect(renderer, &move_rect);
                }

                // Diagonal to the left (attack).
                if (((selection->x - 1) < 8) && ((selection->y - 1) < 8) && 
                    (board[selection->y - 1][selection->x - 1].color == BLACK))
                {
                    move_rect.x = ((selection->x - 1) * 22) + 2;
                    move_rect.y = ((selection->y - 1) * 22) + 2;

                    SDL_RenderFillRect(renderer, &move_rect);
                }

                // Diagonal to the right (attack).
                if (((selection->x + 1) < 8) && ((selection->y - 1) < 8) && 
                    (board[selection->y - 1][selection->x + 1].color == BLACK))
                {
                    move_rect.x = ((selection->x + 1) * 22) + 2;
                    move_rect.y = ((selection->y - 1) * 22) + 2;

                    SDL_RenderFillRect(renderer, &move_rect);
                }
            break;

            case WHITE_ROOK:
                add_possible_moves_long(&possible_moves, selection, board, N);
                add_possible_moves_long(&possible_moves, selection, board, E);
                add_possible_moves_long(&possible_moves, selection, board, S);
                add_possible_moves_long(&possible_moves, selection, board, W);
            break;

            case WHITE_KNIGHT:
                add_simple_possible_move(&possible_moves, selection, board, 2, 1, 0, 0);
                add_simple_possible_move(&possible_moves, selection, board, 2, 0, 1, 0);

                add_simple_possible_move(&possible_moves, selection, board, 1, 2, 0, 0);
                add_simple_possible_move(&possible_moves, selection, board, 0, 2, 0, 1);

                add_simple_possible_move(&possible_moves, selection, board, 1, 0, 2, 0);
                add_simple_possible_move(&possible_moves, selection, board, 0, 0, 2, 1);

                add_simple_possible_move(&possible_moves, selection, board, 0, 1, 0, 2);
                add_simple_possible_move(&possible_moves, selection, board, 0, 0, 1, 2);
            break;

            case WHITE_BISHOP:
                add_possible_moves_long(&possible_moves, selection, board, NE);
                add_possible_moves_long(&possible_moves, selection, board, NW);
                add_possible_moves_long(&possible_moves, selection, board, SE);
                add_possible_moves_long(&possible_moves, selection, board, SW);
            break;

            case WHITE_QUEEN:
                add_possible_moves_long(&possible_moves, selection, board, N);
                add_possible_moves_long(&possible_moves, selection, board, E);
                add_possible_moves_long(&possible_moves, selection, board, S);
                add_possible_moves_long(&possible_moves, selection, board, W);

                add_possible_moves_long(&possible_moves, selection, board, NE);
                add_possible_moves_long(&possible_moves, selection, board, NW);
                add_possible_moves_long(&possible_moves, selection, board, SE);
                add_possible_moves_long(&possible_moves, selection, board, SW);
            break;

            case WHITE_KING:
                add_simple_possible_move(&possible_moves, selection, board, 1, 0, 0, 0);
                add_simple_possible_move(&possible_moves, selection, board, 0, 1, 0, 0);
                add_simple_possible_move(&possible_moves, selection, board, 0, 0, 1, 0);
                add_simple_possible_move(&possible_moves, selection, board, 0, 0, 0, 1);

                add_simple_possible_move(&possible_moves, selection, board, 1, 1, 0, 0);
                add_simple_possible_move(&possible_moves, selection, board, 1, 0, 1, 0);
                add_simple_possible_move(&possible_moves, selection, board, 0, 1, 0, 1);
                add_simple_possible_move(&possible_moves, selection, board, 0, 0, 1, 1);
            break;

            case BLACK_PAWN:
            case BLACK_ROOK:
            case BLACK_KNIGHT:
            case BLACK_BISHOP:
            case BLACK_QUEEN:
            case BLACK_KING:
            default:
            break;

        }

        for (int i = 0; i < possible_moves.count; i++)
        {
            move_rect.x = ((possible_moves.squares[i].x) * 22) + 2;
            move_rect.y = ((possible_moves.squares[i].y) * 22) + 2;

            SDL_RenderFillRect(renderer, &move_rect);
        }
    }

    // Color hovered.
    SDL_Rect hovered_rect;
    hovered_rect.x = (state->hovered->x * 22) + 2;
    hovered_rect.y = (state->hovered->y * 22) + 2;
    hovered_rect.w = 22;
    hovered_rect.h = 22;

    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    SDL_RenderFillRect(renderer, &hovered_rect);

    // Draw board.
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].type != EMPTY)
            {
                SDL_RenderCopy(renderer, board[i][j].texture, NULL, &tile_rect);
            }

            tile_rect.x += 22;
        }
        tile_rect.x = 2;
        tile_rect.y += 22;
    }

    SDL_RenderPresent(renderer);
}

void render_console(SDL_Renderer *renderer, Mouse_State *mouse_state, Game_State *state)
{
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, dog_texture, NULL, NULL);

    if ((mouse_state->pressed == SDL_BUTTON(SDL_BUTTON_LEFT)) && state->selection->selected)
    {
        SDL_Rect mouse_rect;
        mouse_rect.x = mouse_state->x;
        mouse_rect.y = mouse_state->y;
        mouse_rect.w = 22;
        mouse_rect.h = 22;

        SDL_RenderCopy(renderer, state->selection->tile->texture, NULL, &mouse_rect);
    }

    SDL_RenderPresent(renderer);
}

void load_images(SDL_Renderer *renderer) 
{
    SDL_Surface *surface;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (starting_board[i][j].type != EMPTY)
            {
                surface = IMG_Load(starting_board[i][j].texture_path);
                starting_board[i][j].texture = SDL_CreateTextureFromSurface(renderer, surface);
            }
        }
    }

    SDL_Surface *board_surface = SDL_LoadBMP(BOARD_PATH);
    board_texture = SDL_CreateTextureFromSurface(renderer, board_surface);
    SDL_FreeSurface(board_surface);

    SDL_Surface *dog_surface = SDL_LoadBMP(DOG_PATH);
    dog_texture = SDL_CreateTextureFromSurface(renderer, dog_surface);
    SDL_FreeSurface(dog_surface);
}

void update(Tile board[][8], Game_State *state, Mouse_State *mouse_state)
{
    state->hovered->x = (mouse_state->x -2) / 22;
    state->hovered->y = (mouse_state->y -2) / 22;
    state->hovered->tile = &board[state->hovered->y][state->hovered->x];

    if (state->hovered->x > 7) state->hovered->x = 7;
    if (state->hovered->y > 7) state->hovered->y = 7;

    if (state->player_turn)
    {
        if (state->selection->selected) 
        {
            if (mouse_state->pressed == SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                board[state->hovered->y][state->hovered->x].type = state->selection->tile->type;
                board[state->hovered->y][state->hovered->x].color= state->selection->tile->color;
                board[state->hovered->y][state->hovered->x].texture = state->selection->tile->texture;

                board[state->selection->y][state->selection->x].type = EMPTY;
                state->selection->selected = 0;

                state->player_turn = 0;
            }
            else if (mouse_state->pressed == SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                state->selection->selected = 0;
            }
        }
        else
        {
            if ((mouse_state->pressed == SDL_BUTTON(SDL_BUTTON_LEFT)) && (state->hovered->tile->type != EMPTY))
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
        state->player_turn = 1;
    }
}

void get_input(int *quit, Mouse_State *mouse_state, int *console)
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
                        if (*console == 0) {
                            *console = 1;
                        } else {
                            *console = 0;
                        }
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

    printf("Loading ...\n");
    load_images(ren);
    printf("Done!\n");

    // Setup main loop
    int quit = 0;
    int console = 0;
    Mouse_State mouse_state = {0};
    // Tile board[8][8];
    // memcpy(board, starting_board, sizeof(starting_board[0]) * 8 * 8);

    Selection_Info hovered = {0};
    Selection_Info selection = {0};
    Game_State state = {0};
    state.hovered = &hovered;
    state.selection = &selection;
    state.player_turn = 1;

    // Main loop
    const float FPS_INTERVAL = 1.0f;
    Uint64 fps_start, fps_current, fps_frames = 0;

    fps_start = SDL_GetTicks();

    while (!quit)
    {
        SDL_PumpEvents();
        get_input(&quit, &mouse_state, &console);

        if (!quit)
        {
            // TODO(bkaylor): Make the window resizable.
            //
            // int window_w, window_h;
            // SDL_GetWindowSize(win, &window_w, &window_h);
            //

            if (console) 
            {
                render_console(ren, &mouse_state, &state);
            } 
            else 
            {
                update(starting_board, &state, &mouse_state);
                render(ren, starting_board, &state);
            }

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
