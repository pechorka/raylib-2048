#include <stdio.h>
#include <raylib.h>
#include <time.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame(void);
void reset_game(void);

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Hello, World!");
    reset_game();

    #if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    CloseWindow();
    
    return 0;
}


#define BOARD_DIMENTION 4
#define CELL_SIZE 60
#define CELL_TEXT_SIZE 20
#define BOARD_SIZE (BOARD_DIMENTION * CELL_SIZE)

int board[BOARD_DIMENTION][BOARD_DIMENTION];
int score;

int freeSlotsCount;
int maxCurrent2Power;

int bestScore;

int random_2_power(int max) {
    int power = rand() % (max) + 1;
    return 1 << power; 
}

int place_at_random_free_slot(int value) {
    if (freeSlotsCount == 0) {
        printf("No free slots\n");
        return 0;
    }

    int nth_slot = rand() % freeSlotsCount+1;
    printf("free_slots_count: %d\n", freeSlotsCount);
    printf("nth_slot: %d\n", nth_slot);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0 && --nth_slot == 0) {
                board[i][j] = value;
                freeSlotsCount--;
                printf("Placed %d at %d, %d\n", value, i, j);
                return 1;
            }
        }
    }
    printf("Failed to place %d, %d\n", value, nth_slot);
    return -1; // should never happen
}

void reset_game(void) {
    // reseed random number generator
    srand(time(NULL));

    for (int i = 0; i < BOARD_DIMENTION; i++) {
        for (int j = 0; j < BOARD_DIMENTION; j++) {
            board[i][j] = 0;
        }
    }
    freeSlotsCount = BOARD_DIMENTION * BOARD_DIMENTION;
    maxCurrent2Power = 1;

    for (int i = 0; i < 2; i++) {
        place_at_random_free_slot(random_2_power(maxCurrent2Power));
    }

    if (score > bestScore) {
        bestScore = score;
    }
    score = 0;
}

Color get_color(int value) {
    if (value == 0) {
        return (Color){200, 200, 200, 255};
    }
    int r = 255;
    int g = 255;
    int b = 255;
    for (int i = value; i > 0 && value > 1; i--, value >>= 1) {
        if (i%2 == 0) {
            r -= 10;
        } else if (i%3 == 0) {
            g -= 10;
        } else {
            b -= 10;
        }
    }
    return (Color){r, g, b, 255};
}

void render_board(void) {
    // render board border
    Rectangle board_rect = {
        .x = (GetScreenWidth() - BOARD_SIZE) / 2,
        .y = (GetScreenHeight() - BOARD_SIZE) / 2,
        .width = BOARD_SIZE,
        .height = BOARD_SIZE
    };
    DrawRectangleLinesEx(board_rect, 2, BLACK);
    // render cells
    for (int i = 0; i < BOARD_DIMENTION; i++) {
        for (int j = 0; j < BOARD_DIMENTION; j++) {
            int value = board[i][j];
            int x = board_rect.x + j * CELL_SIZE;
            int y = board_rect.y + i * CELL_SIZE;
            Rectangle cell_rect = {
                .x = x,
                .y = y,
                .width = CELL_SIZE,
                .height = CELL_SIZE
            };
            Color cell_color = get_color(value);
            DrawRectangleRec(cell_rect, cell_color);
            DrawRectangleLinesEx(cell_rect, 2, BLACK);
            if (value == 0) {
                continue;
            }
            char text[10];
            sprintf(text, "%d", value);
            int textWidth = MeasureText(text, CELL_TEXT_SIZE);
            // find center of the cell
            int textX = x + CELL_SIZE / 2 - textWidth / 2;
            int textY = y + CELL_SIZE / 2 - CELL_TEXT_SIZE / 2;
            DrawText(text, textX, textY, CELL_TEXT_SIZE, BLACK);
        }
    }

    // render score above the board
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);
    DrawText(scoreText, board_rect.x, board_rect.y - 40, 30, BLACK);
    // draw best score above score
    char bestScoreText[50];
    sprintf(bestScoreText, "Best: %d", bestScore);
    DrawText(bestScoreText, board_rect.x, board_rect.y - 80, 30, BLACK);
    
}

void move_cells_left() {
    for (int i = 0; i < BOARD_DIMENTION; i++) {
        for (int j = 0; j < BOARD_DIMENTION; j++) {
            if (board[i][j] == 0) {
                continue;
            }

            int k = j;
            while (k > 0 && board[i][k-1] == 0) {
                k--;
            }
            if (k != j) {
                board[i][k] = board[i][j];
                board[i][j] = 0;
            }
        }
    }
}

void merge_cells_left() {
    for (int i = 0; i < BOARD_DIMENTION; i++) {
        for (int j = 0; j < BOARD_DIMENTION-1; j++) {
            if (!board[i][j]) {
                continue;
            }
            if (board[i][j] == board[i][j+1]) {
                board[i][j] *= 2;
                board[i][j+1] = 0;

                score += board[i][j];
                freeSlotsCount++;
                if (board[i][j] > (1 << maxCurrent2Power)) {
                    maxCurrent2Power++;
                }
            }
        }
    }
}

void move_cells_right() {
    for (int i = 0; i < BOARD_DIMENTION; i++) {
        for (int j = BOARD_DIMENTION-1; j >= 0; j--) {
            if (board[i][j] == 0) {
                continue;
            }

            int k = j;
            while (k < BOARD_DIMENTION-1 && board[i][k+1] == 0) {
                k++;
            }
            if (k != j) {
                board[i][k] = board[i][j];
                board[i][j] = 0;
            }
        }
    }
}

void merge_cells_right() {
    for (int i = 0; i < BOARD_DIMENTION; i++) {
        for (int j = BOARD_DIMENTION-1; j > 0; j--) {
            if (!board[i][j]) {
                continue;
            }
            if (board[i][j] == board[i][j-1]) {
                board[i][j] *= 2;
                board[i][j-1] = 0;

                score += board[i][j];
                freeSlotsCount++;
                if (board[i][j] > (1 << maxCurrent2Power)) {
                    maxCurrent2Power++;
                }
            }
        }
    }
}

void transpose_board() {
    for (int i = 0; i < BOARD_DIMENTION; i++) {
        for (int j = i+1; j < BOARD_DIMENTION; j++) {
            int temp = board[i][j];
            board[i][j] = board[j][i];
            board[j][i] = temp;
        }
    }
}

int handle_input(void) {
    if (IsKeyPressed(KEY_R)) {
        reset_game();
        return 0;
    }

    switch (GetKeyPressed())
    {
    case KEY_LEFT:
        move_cells_left();
        merge_cells_left();
        move_cells_left();
        return 1;
    case KEY_RIGHT:
        move_cells_right();
        merge_cells_right();
        move_cells_right();
        return 1;
    case KEY_UP:
        transpose_board();
        move_cells_left();
        merge_cells_left();
        move_cells_left();
        transpose_board();
        return 1;
    case KEY_DOWN:
        transpose_board();
        move_cells_right();
        merge_cells_right();
        move_cells_right();
        transpose_board();
        return 1;
    default:
        break;
    }

    return 0;
}

void UpdateDrawFrame(void) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (freeSlotsCount == 0) {
        DrawText("Game Over. Press 'R' to restart.", 190, 200, 40, MAROON);
        return;
    }

    if (handle_input()) place_at_random_free_slot(random_2_power(maxCurrent2Power));
    
    render_board();

    EndDrawing();
}

