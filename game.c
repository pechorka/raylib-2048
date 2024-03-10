#include <stdio.h>
#include <raylib.h>
#include <time.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

Rectangle screenRect = {
    .x = 0,
    .y = 0,
    .width = SCREEN_WIDTH,
    .height = SCREEN_HEIGHT,
};

#define BOARD_DIMENTION 4
#define CELL_SIZE 60
#define CELL_TEXT_SIZE 20

#define BOARD_X (SCREEN_WIDTH - BOARD_DIMENTION * CELL_SIZE) / 2
#define BOARD_Y (SCREEN_HEIGHT - BOARD_DIMENTION * CELL_SIZE) / 2
#define BOARD_WIDTH (BOARD_DIMENTION * CELL_SIZE)
#define BOARD_HEIGHT BOARD_WIDTH

Rectangle boardRect = {
    .x = BOARD_X,
    .y = BOARD_Y,
    .width = BOARD_WIDTH,
    .height = BOARD_HEIGHT,
};

#define CONTROL_SIZE 40
#define CONTROL_TEXT_SIZE 30
#define CONTROL_PADDING 10
#define CONTROL_SIZE_WITH_PADDING (CONTROL_SIZE + CONTROL_PADDING * 2)

#define CONTROL_PANEL_X (SCREEN_WIDTH - CONTROL_PANEL_WIDTH) / 2
#define CONTROL_PANEL_Y (BOARD_Y + BOARD_HEIGHT + 20)
#define CONTROL_PANEL_WIDTH (CONTROL_SIZE_WITH_PADDING * 3)
#define CONTROL_PANEL_HEIGHT (CONTROL_SIZE_WITH_PADDING * 2)

Rectangle controlRect = {
    .x = CONTROL_PANEL_X,
    .y = CONTROL_PANEL_Y,
    .width = CONTROL_PANEL_WIDTH,
    .height = CONTROL_PANEL_HEIGHT,
};

Rectangle upControlRect = {
    .x = (CONTROL_PANEL_WIDTH - CONTROL_SIZE) / 2 + CONTROL_PANEL_X,
    .y = CONTROL_PANEL_Y + CONTROL_PADDING,
    .width = CONTROL_SIZE,
    .height = CONTROL_SIZE,
};

Rectangle downControlRect = {
    .x = (CONTROL_PANEL_WIDTH - CONTROL_SIZE) / 2 + CONTROL_PANEL_X,
    .y = CONTROL_PANEL_Y + CONTROL_PANEL_HEIGHT - CONTROL_SIZE - CONTROL_PADDING,
    .width = CONTROL_SIZE,
    .height = CONTROL_SIZE,
};

Rectangle leftControlRect = {
    .x = CONTROL_PANEL_X + CONTROL_PADDING,
    .y = (CONTROL_PANEL_HEIGHT - CONTROL_SIZE) / 2 + CONTROL_PANEL_Y,
    .width = CONTROL_SIZE,
    .height = CONTROL_SIZE,
};

Rectangle rightControlRect = {
    .x = CONTROL_PANEL_X + CONTROL_PANEL_WIDTH - CONTROL_SIZE - CONTROL_PADDING,
    .y = (CONTROL_PANEL_HEIGHT - CONTROL_SIZE) / 2 + CONTROL_PANEL_Y,
    .width = CONTROL_SIZE,
    .height = CONTROL_SIZE,
};

void UpdateDrawFrame(void);
void reset_game(void);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello, World!");
    reset_game();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    CloseWindow();

    return 0;
}

enum MoveState
{
    MOVE_STATE_IDLE,
    MOVE_STATE_MOVE_BEFORE_MERGE,
    MOVE_STATE_MERGING,
    MOVE_STATE_MOVE_AFTER_MERGE,
    MOVE_STATE_SPAWN
};

int board[BOARD_DIMENTION][BOARD_DIMENTION];
int score;

int currentDirection;
int moveProgress;
enum MoveState moveState;

int freeSlotsCount;
int maxCurrent2Power;

int bestScore;

int random_2_power(int max)
{
    int power = rand() % (max) + 1;
    return 1 << power;
}

int place_at_random_free_slot(int value)
{
    if (freeSlotsCount == 0)
    {
        TraceLog(LOG_WARNING, "No free slots to place %d", value);
        return 0;
    }

    int nth_slot = rand() % freeSlotsCount + 1;
    for (int i = 0; i < BOARD_DIMENTION; i++)
    {
        for (int j = 0; j < BOARD_DIMENTION; j++)
        {
            if (board[i][j] == 0 && --nth_slot == 0)
            {
                board[i][j] = value;
                freeSlotsCount--;
                return 1;
            }
        }
    }
    TraceLog(LOG_WARNING, "Failed to place %d", value);
    return -1; // should never happen
}

void reset_game(void)
{
    // reseed random number generator
    srand(time(NULL));

    for (int i = 0; i < BOARD_DIMENTION; i++)
    {
        for (int j = 0; j < BOARD_DIMENTION; j++)
        {
            board[i][j] = 0;
        }
    }
    freeSlotsCount = BOARD_DIMENTION * BOARD_DIMENTION;
    maxCurrent2Power = 1;

    for (int i = 0; i < 2; i++)
    {
        place_at_random_free_slot(random_2_power(maxCurrent2Power));
    }

    score = 0;

    currentDirection = 0;
    moveProgress = 0;
    moveState = MOVE_STATE_IDLE;
}

Color get_color(int value)
{
    if (value == 0)
    {
        return (Color){200, 200, 200, 255};
    }
    int r = 255;
    int g = 255;
    int b = 255;
    for (int i = value; i > 0 && value > 1; i--, value >>= 1)
    {
        if (i % 2 == 0)
        {
            r -= 10;
        }
        else if (i % 3 == 0)
        {
            g -= 10;
        }
        else
        {
            b -= 10;
        }
    }
    return (Color){r, g, b, 255};
}

Rectangle render_text_at_center_of_rect(char *text, Rectangle rect, int fontSize, Color color)
{
    int textWidth = MeasureText(text, fontSize);
    int textX = rect.x + rect.width / 2 - textWidth / 2;
    int textY = rect.y + rect.height / 2 - fontSize / 2;
    DrawText(text, textX, textY, fontSize, color);

    return (Rectangle){textX, textY, textWidth, fontSize};
}

void render_game(void)
{
    DrawRectangleLinesEx(boardRect, 2, BLACK);
    // render cells
    for (int i = 0; i < BOARD_DIMENTION; i++)
    {
        for (int j = 0; j < BOARD_DIMENTION; j++)
        {
            int value = board[i][j];
            int x = boardRect.x + j * CELL_SIZE;
            int y = boardRect.y + i * CELL_SIZE;
            Rectangle cell_rect = {
                .x = x,
                .y = y,
                .width = CELL_SIZE,
                .height = CELL_SIZE};
            Color cell_color = get_color(value);
            DrawRectangleRec(cell_rect, cell_color);
            DrawRectangleLinesEx(cell_rect, 2, BLACK);
            if (value == 0)
            {
                continue;
            }
            char text[10];
            sprintf(text, "%d", value);
            render_text_at_center_of_rect(text, cell_rect, CELL_TEXT_SIZE, BLACK);
        }
    }

    // render score above the board
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);
    DrawText(scoreText, boardRect.x, boardRect.y - 40, 30, BLACK);
    // draw best score above score
    char bestScoreText[50];
    sprintf(bestScoreText, "Best: %d", bestScore);
    DrawText(bestScoreText, boardRect.x, boardRect.y - 80, 30, BLACK);

    // render control panel
    DrawRectangleLinesEx(controlRect, 2, BLACK);

    DrawRectangleLinesEx(upControlRect, 2, BLACK);
    render_text_at_center_of_rect("^", upControlRect, CONTROL_TEXT_SIZE, BLACK);

    DrawRectangleLinesEx(downControlRect, 2, BLACK);
    render_text_at_center_of_rect("v", downControlRect, CONTROL_TEXT_SIZE, BLACK);

    DrawRectangleLinesEx(leftControlRect, 2, BLACK);
    render_text_at_center_of_rect("<", leftControlRect, CONTROL_TEXT_SIZE, BLACK);

    DrawRectangleLinesEx(rightControlRect, 2, BLACK);
    render_text_at_center_of_rect(">", rightControlRect, CONTROL_TEXT_SIZE, BLACK);
}

void move_cells_left()
{
    for (int i = 0; i < BOARD_DIMENTION; i++)
    {
        for (int j = 0; j < BOARD_DIMENTION; j++)
        {
            if (board[i][j] == 0)
            {
                continue;
            }
            if (j - 1 < 0)
            {
                continue;
            }
            if (board[i][j - 1] == 0)
            {
                board[i][j - 1] = board[i][j];
                board[i][j] = 0;
            }
        }
    }
}

void merge_cells_left()
{
    for (int i = 0; i < BOARD_DIMENTION; i++)
    {
        for (int j = 0; j < BOARD_DIMENTION - 1; j++)
        {
            if (!board[i][j])
            {
                continue;
            }
            if (board[i][j] == board[i][j + 1])
            {
                board[i][j] *= 2;
                board[i][j + 1] = 0;

                score += board[i][j];
                freeSlotsCount++;
                if (board[i][j] > (1 << maxCurrent2Power))
                {
                    maxCurrent2Power++;
                }
            }
        }
    }
}

void move_cells_right()
{
    for (int i = 0; i < BOARD_DIMENTION; i++)
    {
        for (int j = BOARD_DIMENTION - 1; j >= 0; j--)
        {
            if (board[i][j] == 0)
            {
                continue;
            }
            if (j + 1 == BOARD_DIMENTION)
            {
                continue;
            }
            if (board[i][j + 1] == 0)
            {
                board[i][j + 1] = board[i][j];
                board[i][j] = 0;
            }
        }
    }
}

void merge_cells_right()
{
    for (int i = 0; i < BOARD_DIMENTION; i++)
    {
        for (int j = BOARD_DIMENTION - 1; j > 0; j--)
        {
            if (!board[i][j])
            {
                continue;
            }
            if (board[i][j] == board[i][j - 1])
            {
                board[i][j] *= 2;
                board[i][j - 1] = 0;

                score += board[i][j];
                freeSlotsCount++;
                if (board[i][j] > (1 << maxCurrent2Power))
                {
                    maxCurrent2Power++;
                }
            }
        }
    }
}

void transpose_board()
{
    for (int i = 0; i < BOARD_DIMENTION; i++)
    {
        for (int j = i + 1; j < BOARD_DIMENTION; j++)
        {
            int temp = board[i][j];
            board[i][j] = board[j][i];
            board[j][i] = temp;
        }
    }
}

int is_button_pressed(Rectangle rect)
{
    Vector2 mousePos = GetMousePosition();
    return IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, rect);
}

enum MoveState get_state()
{
    // current move is not finished
    if (moveProgress > 0)
        return moveState;

    switch (moveState)
    {
    case MOVE_STATE_IDLE: // nothing to do
        break;
    case MOVE_STATE_MOVE_BEFORE_MERGE:
        moveState = MOVE_STATE_MERGING;
        moveProgress = BOARD_DIMENTION;
        break;
    case MOVE_STATE_MERGING:
        moveState = MOVE_STATE_MOVE_AFTER_MERGE;
        moveProgress = BOARD_DIMENTION;
        break;
    case MOVE_STATE_MOVE_AFTER_MERGE:
        moveState = MOVE_STATE_SPAWN;
        moveProgress = 1;
        break;
    case MOVE_STATE_SPAWN:
        moveState = MOVE_STATE_IDLE;
        currentDirection = 0;
        break;
    }

    return moveState;
}

int get_direction()
{
    if (moveProgress > 0)
        return currentDirection;

    if (IsKeyDown(KEY_LEFT) || is_button_pressed(leftControlRect))
        currentDirection = KEY_LEFT;
    if (IsKeyDown(KEY_RIGHT) || is_button_pressed(rightControlRect))
        currentDirection = KEY_RIGHT;
    if (IsKeyDown(KEY_UP) || is_button_pressed(upControlRect))
        currentDirection = KEY_UP;
    if (IsKeyDown(KEY_DOWN) || is_button_pressed(downControlRect))
        currentDirection = KEY_DOWN;

    if (currentDirection != 0)
    {
        moveState = MOVE_STATE_MOVE_BEFORE_MERGE;
        moveProgress = BOARD_DIMENTION;
    }

    return currentDirection;
}

void move(void)
{
    enum MoveState state = get_state();
    int direction = get_direction();

    if (state == MOVE_STATE_IDLE)
    {
        return;
    }

    moveProgress--;

    if (state == MOVE_STATE_SPAWN)
    {
        place_at_random_free_slot(random_2_power(maxCurrent2Power));
        return;
    }

    if (direction == KEY_LEFT)
    {
        if (state == MOVE_STATE_MERGING)
        {
            merge_cells_left();
        }
        else
        {
            move_cells_left();
        }
        return;
    }

    if (direction == KEY_RIGHT)
    {
        if (state == MOVE_STATE_MERGING)
        {
            merge_cells_right();
        }
        else
        {
            move_cells_right();
        }
        return;
    }

    if (direction == KEY_UP)
    {
        transpose_board();
        if (state == MOVE_STATE_MERGING)
        {
            merge_cells_left();
        }
        else
        {
            move_cells_left();
        }
        transpose_board();
        return;
    }

    if (direction == KEY_DOWN)
    {
        transpose_board();
        if (state == MOVE_STATE_MERGING)
        {
            merge_cells_right();
        }
        else
        {
            move_cells_right();
        }
        transpose_board();
        return;
    }

    return;
}

void draw_game_over_screen(void) {
    Rectangle borders = render_text_at_center_of_rect("Game Over", screenRect, 50, BLACK);
    // draw restart button to the right of the text
    char* restartText = "Restart";
    int restartFontSize = 30;
    int restartTextWidth = MeasureText(restartText, restartFontSize);
    Rectangle restartButton = {
        .x = borders.x + borders.width + 20,
        .y = borders.y,
        .width = restartTextWidth + 20,
        .height = restartFontSize + 20,
    };

    Color restartButtonColor = BLACK;

    if (CheckCollisionPointRec(GetMousePosition(), restartButton))
    {
        restartButtonColor = GREEN;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            reset_game();
            return;
        }
    }

    DrawRectangleLinesEx(restartButton, 2, restartButtonColor);
    render_text_at_center_of_rect("Restart", restartButton, restartFontSize, restartButtonColor);

    // draw score below "Game Over"
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);
    int scoreX = borders.x;
    int scoreY = borders.y + borders.height + 20;
    DrawText(scoreText, scoreX, scoreY, 30, BLACK);

    // draw best score below score
    char bestScoreText[50];
    sprintf(bestScoreText, "Best: %d", bestScore);
    int bestScoreX = borders.x;
    int bestScoreY = scoreY + 40;
    DrawText(bestScoreText, bestScoreX, bestScoreY, 30, BLACK);
}

void UpdateDrawFrame(void)
{
    if (IsKeyPressed(KEY_R))
    {
        reset_game();
    }
    
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (freeSlotsCount == 0)
    {
        if (score > bestScore)
        {
            bestScore = score;
        }
        draw_game_over_screen();
    }
    else
    {
        move();
        render_game();
    }

    EndDrawing();
}
