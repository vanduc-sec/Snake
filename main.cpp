#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <string>
#include "button.hpp"

using namespace std;

Color green = {173, 204, 96, 255};
Color darkgreen = {43, 51, 24, 255};

int kichThuocO = 25; //cellsize
int SoOtrongMoiHangCot = 20; //cellcount
int offset = 62.5;

double lastUpdateTime = 0;

Texture2D backgroundTexture;
Texture2D snakeHeadTexture;
Texture2D snakeBodyTexture;
Texture2D foodTextures[5];
int totalFoodTypes = 5;

// === Thêm nhạc nền menu ===
Music backgroundMusic;
bool musicPlaying = false;

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * kichThuocO, offset + y * kichThuocO, (float)kichThuocO, (float)kichThuocO};

            if (i == 0) // Head
            {
                float rotation = 0.0f;
                if (direction.x == 1)
                    rotation = 0.0f;
                else if (direction.x == -1)
                    rotation = 180.0f;
                else if (direction.y == -1)
                    rotation = 270.0f;
                else if (direction.y == 1)
                    rotation = 90.0f;

                Vector2 origin = {kichThuocO / 2.0f, kichThuocO / 2.0f};
                Vector2 position = {segment.x + kichThuocO / 2.0f, segment.y + kichThuocO / 2.0f};
                DrawTexturePro(snakeHeadTexture,
                               Rectangle{0, 0, (float)snakeHeadTexture.width, (float)snakeHeadTexture.height},
                               Rectangle{position.x, position.y, (float)kichThuocO, (float)kichThuocO},
                               origin, rotation, WHITE);
            }
            else
            {
                float rotation = 0.0f;
                Vector2 currentDirection = {0, 0};
                Vector2 prevSegment = body[i - 1];
                Vector2 currentSegment = body[i];
                currentDirection.x = prevSegment.x - currentSegment.x;
                currentDirection.y = prevSegment.y - currentSegment.y;

                if (currentDirection.x == 1)
                    rotation = 0.0f;
                else if (currentDirection.x == -1)
                    rotation = 180.0f;
                else if (currentDirection.y == -1)
                    rotation = 270.0f;
                else if (currentDirection.y == 1)
                    rotation = 90.0f;

                Vector2 origin = {kichThuocO / 2.0f, kichThuocO / 2.0f};
                Vector2 position = {segment.x + kichThuocO / 2.0f, segment.y + kichThuocO / 2.0f};
                DrawTexturePro(snakeBodyTexture,
                               Rectangle{0, 0, (float)snakeBodyTexture.width, (float)snakeBodyTexture.height},
                               Rectangle{position.x, position.y, (float)kichThuocO, (float)kichThuocO},
                               origin, rotation, WHITE);
            }
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (!addSegment)
        {
            body.pop_back();
        }
        addSegment = false;
    }

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{
public:
    Vector2 position;
    int foodType;

    Food(deque<Vector2> snakeBody)
    {
        position = GenerateRandomPos(snakeBody);
        foodType = GetRandomValue(0, totalFoodTypes - 1);
    }

    void Draw()
    {
        float scale = (float)kichThuocO / 28.0f;
        Vector2 pos = {offset + position.x * kichThuocO, offset + position.y * kichThuocO};
        DrawTextureEx(foodTextures[foodType], pos, 0.0f, scale, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, SoOtrongMoiHangCot - 1);
        float y = GetRandomValue(0, SoOtrongMoiHangCot - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }

    void Regenerate(deque<Vector2> snakeBody)
    {
        position = GenerateRandomPos(snakeBody);
        foodType = GetRandomValue(0, totalFoodTypes - 1);
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;
    bool isDead = false;

    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.Regenerate(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == SoOtrongMoiHangCot || snake.body[0].x == -1 ||
            snake.body[0].y == SoOtrongMoiHangCot || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.Regenerate(snake.body);
        running = false;
        isDead = true;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    void Restart()
    {
        snake.Reset();
        food.Regenerate(snake.body);
        score = 0;
        isDead = false;
        running = true;
    }
};

enum GameScreen
{
    MENU,
    GAME
};

int main()
{
    GameScreen currentScreen = MENU;
    cout << "Starting the game..." << endl;

    int windowWidth = 2 * offset + kichThuocO * SoOtrongMoiHangCot;
    int windowHeight = 2 * offset + kichThuocO * SoOtrongMoiHangCot;

    InitWindow(windowWidth, windowHeight, "Snake Game");
    SetTargetFPS(60);

    Texture2D menubackground = LoadTexture("Graphics/background.png");
    Button startButton{"Graphics/start.png", {250, 420}, 3.5};
    Button exitButton{"Graphics/exit.png", {250, 480}, 3.5};

    Image backgroundImage = LoadImage("Graphics/san.png");
    backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);

    Image headImage = LoadImage("Graphics/dau.png");
    snakeHeadTexture = LoadTextureFromImage(headImage);
    UnloadImage(headImage);

    Image bodyImage = LoadImage("Graphics/than.png");
    snakeBodyTexture = LoadTextureFromImage(bodyImage);
    UnloadImage(bodyImage);

    for (int i = 0; i < totalFoodTypes; i++)
    {
        string foodPath = "Graphics/Food" + to_string(i + 1) + ".png";
        Image foodImage = LoadImage(foodPath.c_str());
        foodTextures[i] = LoadTextureFromImage(foodImage);
        UnloadImage(foodImage);
    }

    Game game = Game();

    // Load nhạc menu một lần duy nhất
    backgroundMusic = LoadMusicStream("Sounds/nhacnen.mp3");

    while (!WindowShouldClose())
    {
        Vector2 mousePosition = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        // Xử lý nhạc theo màn hình
        if (currentScreen == MENU)
        {
            if (!musicPlaying)
            {
                PlayMusicStream(backgroundMusic);
                musicPlaying = true;
            }
            UpdateMusicStream(backgroundMusic);
        }
        else if (currentScreen == GAME)
        {
            if (musicPlaying)
            {
                StopMusicStream(backgroundMusic);
                musicPlaying = false;
            }
        }

        BeginDrawing();

        if (currentScreen == MENU)
        {
            ClearBackground(green);

            float scaleX = (float)windowWidth / menubackground.width;
            float scaleY = (float)windowHeight / menubackground.height;
            float scale = fmaxf(scaleX, scaleY);
            DrawTextureEx(menubackground, Vector2{0, 0}, 0.0f, scale, WHITE);

            startButton.Draw();
            exitButton.Draw();

            if (startButton.isPressed(mousePosition, mousePressed))
            {
                game.Restart();
                currentScreen = GAME;
            }

            if (exitButton.isPressed(mousePosition, mousePressed))
            {
                break;
            }
        }
        else if (currentScreen == GAME)
        {
            if (eventTriggered(0.2))
            {
                game.Update();
            }

            if (game.running)
            {
                if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
                    game.snake.direction = {0, -1};
                if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
                    game.snake.direction = {0, 1};
                if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
                    game.snake.direction = {-1, 0};
                if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
                    game.snake.direction = {1, 0};
            }

            if (IsKeyPressed(KEY_Q))
            {
                currentScreen = MENU;
                game.running = false;
                game.isDead = false;
            }

            if (game.isDead && IsKeyPressed(KEY_R))
            {
                game.Restart();
            }

            float scaleX = (float)windowWidth / backgroundTexture.width;
            float scaleY = (float)windowHeight / backgroundTexture.height;
            float scale = fmaxf(scaleX, scaleY);

            DrawTextureEx(backgroundTexture, Vector2{0, 0}, 0.0f, scale, WHITE);

            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)SoOtrongMoiHangCot * kichThuocO + 10, (float)SoOtrongMoiHangCot * kichThuocO + 10}, 5, darkgreen);
            DrawText("Snake Game", offset - 5, 20, 40, darkgreen);
            DrawText(TextFormat("Score: %i", game.score), offset - 5, offset + 5 + SoOtrongMoiHangCot * kichThuocO, 40, darkgreen);

            game.Draw();

            if (game.isDead)
            {
                DrawRectangle(0, 0, windowWidth, windowHeight, Fade(BLACK, 0.7f));
                DrawText("GAME OVER!", windowWidth / 2 - 120, windowHeight / 2 - 60, 40, RED);
                DrawText(TextFormat("Final Score: %i", game.score), windowWidth / 2 - 80, windowHeight / 2 - 10, 20, WHITE);
                DrawText("Press R to Restart", windowWidth / 2 - 90, windowHeight / 2 + 20, 20, WHITE);
                DrawText("Press Q to Menu", windowWidth / 2 - 85, windowHeight / 2 + 50, 20, WHITE);
            }
            else if (!game.running && !game.isDead)
            {
                DrawText("Press Arrow Keys to Start", windowWidth / 2 - 120, windowHeight / 2, 20, darkgreen);
                DrawText("Q - Back to Menu", windowWidth / 2 - 80, windowHeight / 2 + 30, 16, darkgreen);
            }
        }

        EndDrawing();
    }

    UnloadTexture(menubackground);
    UnloadTexture(backgroundTexture);
    UnloadTexture(snakeHeadTexture);
    UnloadTexture(snakeBodyTexture);
    for (int i = 0; i < totalFoodTypes; i++)
    {
        UnloadTexture(foodTextures[i]);
    }

    UnloadMusicStream(backgroundMusic);
    CloseWindow();
    return 0;
}
