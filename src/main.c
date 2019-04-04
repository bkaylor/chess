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

typedef enum Tile_Enum {
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
} Tile_Type;

typedef struct Piece_Struct {
    Tile_Type type;
    char *texture_path;
    SDL_Texture *texture;
} Piece;

typedef struct Mouse_State_Struct {
    int x;
    int y;
    int pressed;
} Mouse_State;

typedef struct Selection_Info_Struct {
    int x;
    int y;
    int selected;
    Piece *piece;
} Selection_Info;

Piece starting_board[8][8] = {
    {
        {BLACK_ROOK, BLACK_ROOK_PATH, NULL}, 
        {BLACK_BISHOP, BLACK_BISHOP_PATH, NULL}, 
        {BLACK_KNIGHT, BLACK_KNIGHT_PATH, NULL},
        {BLACK_QUEEN, BLACK_QUEEN_PATH, NULL},
        {BLACK_KING, BLACK_KING_PATH, NULL},
        {BLACK_KNIGHT, BLACK_KNIGHT_PATH, NULL},
        {BLACK_BISHOP, BLACK_BISHOP_PATH, NULL},
        {BLACK_ROOK, BLACK_ROOK_PATH, NULL}
    },
    {
        {BLACK_PAWN, BLACK_PAWN_PATH, NULL}, 
        {BLACK_PAWN, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK_PAWN_PATH, NULL},
        {BLACK_PAWN, BLACK_PAWN_PATH, NULL}
    },
    {
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},    
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},    
    },
    {
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},    
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},    
    },
    {
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},    
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},    
    },
    {
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},    
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},
        {EMPTY, "", NULL},    
    },
    {
        {WHITE_PAWN, WHITE_PAWN_PATH, NULL}, 
        {WHITE_PAWN, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE_PAWN_PATH, NULL},
        {WHITE_PAWN, WHITE_PAWN_PATH, NULL}
    },
    {
        {WHITE_ROOK, WHITE_ROOK_PATH, NULL}, 
        {WHITE_BISHOP, WHITE_BISHOP_PATH, NULL}, 
        {WHITE_KNIGHT, WHITE_KNIGHT_PATH, NULL},
        {WHITE_KING, WHITE_KING_PATH, NULL},
        {WHITE_QUEEN, WHITE_QUEEN_PATH, NULL},
        {WHITE_KNIGHT, WHITE_KNIGHT_PATH, NULL},
        {WHITE_BISHOP, WHITE_BISHOP_PATH, NULL},
        {WHITE_ROOK, WHITE_ROOK_PATH, NULL}
    },
};

SDL_Texture *board_texture;
SDL_Texture *dog_texture;

void render(SDL_Renderer *renderer, Piece board[][8], Selection_Info *selection)
{
    SDL_RenderClear(renderer);

    // Draw background.
    SDL_RenderCopy(renderer, board_texture, NULL, NULL);

    SDL_Rect piece_rect;
    piece_rect.x = 0 + 2;
    piece_rect.y = 0 + 2;
    piece_rect.w = 22;
    piece_rect.h = 22;

    // Color selection.
    SDL_Rect selection_rect;
    selection_rect.x = (selection->x * 22) + 2;
    selection_rect.y = (selection->y * 22) + 2;
    selection_rect.w = 22;
    selection_rect.h = 22;

    if (selection->selected)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 128, 128);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    }

    SDL_RenderFillRect(renderer, &selection_rect);

    // Color available moves. 

    // Draw board.
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            SDL_RenderCopy(renderer, board[i][j].texture, NULL, &piece_rect);
            piece_rect.x += 22;
        }
        piece_rect.x = 2;
        piece_rect.y += 22;
    }

    SDL_RenderPresent(renderer);
}

void render_console(SDL_Renderer *renderer, Mouse_State *mouse_state)
{
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, dog_texture, NULL, NULL);

    if (mouse_state->pressed == SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        SDL_Rect mouse_rect;
        mouse_rect.x = mouse_state->x;
        mouse_rect.y = mouse_state->y;
        mouse_rect.w = 22;
        mouse_rect.h = 22;

        SDL_RenderCopy(renderer, starting_board[0][1].texture, NULL, &mouse_rect);
    }

    SDL_RenderPresent(renderer);
}

void load_images(SDL_Renderer *renderer) 
{
    //
    // SDL_Surface *board_surface = IMG_Load(board_texture_path);
    // board_texture = SDL_CreateTextureFromSurface(renderer, board_surface);
    // SDL_FreeSurface(board_surface);
    //

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

void update(Piece board[][8], Selection_Info *selection, Mouse_State *mouse_state)
{
    selection->x = (mouse_state->x - 2) / 22;
    selection->y = (mouse_state->y - 2) / 22;
    selection->piece = &board[selection->x][selection->y];

    if (selection->x > 7) selection->x = 7;
    if (selection->y > 7) selection->y = 7;

    if (mouse_state->pressed == SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        selection->selected = 1;
    }
    else
    {
        selection->selected = 0;
    }
}

void get_input(int *quit, Mouse_State *mouse_state, int *console)
{
    // Get mouse info.
    mouse_state->pressed = SDL_GetMouseState(&mouse_state->x, &mouse_state->y);

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
    // Piece board[8][8];
    // memcpy(board, starting_board, sizeof(starting_board[0]) * 8 * 8);
    Selection_Info selection = {0};

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
            //
            // int window_w, window_h;
            // SDL_GetWindowSize(win, &window_w, &window_h);
            //

            update(starting_board, &selection, &mouse_state);

            if (console) {
                render_console(ren, &mouse_state);
            } else {
                render(ren, starting_board, &selection);
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
