#include <iostream>

#include <raylib.h>      // Thư viện đồ họa và game
// Thư viện đồ họa Raylib - dùng để vẽ đồ họa 2D/3D, xử lý input, âm thanh
// Trong game rắn sẽ dùng để: vẽ màn hình, vẽ rắn, vẽ mồi, xử lý phím bấm

#include <deque>         // Cấu trúc dữ liệu deque để lưu thân rắn
// Deque (Double-ended queue) - cấu trúc dữ liệu hàng đợi 2 đầu
// Dùng để lưu các đoạn thân rắn, có thể thêm/xóa ở cả 2 đầu một cách hiệu quả
// Khi rắn di chuyển: thêm phần đầu mới, xóa phần đuôi cũ

#include <raymath.h>     // Thư viện toán học cho Vector2
// Thư viện toán học của Raylib
// Cung cấp các hàm xử lý Vector2 (tọa độ 2D), kiểm tra va chạm, tính toán góc...

#include <string>
#include "button.hpp"    // Class Button tự định nghĩa
// File header tự định nghĩa chứa class Button
// Có thể dùng để tạo các nút bấm trong menu game (Start, Pause, Restart...)

using namespace std;

//CẤU HÌNH MÀU SẮC
Color green = {173, 204, 96, 255};
Color darkgreen = {43, 51, 24, 255};
// Định nghĩa 2 màu chủ đạo của game (kiểu retro giống game rắn cổ điển)
// Format: {R, G, B, Alpha} - giá trị từ 0-255
// Dùng để tạo bảng màu cho background, rắn, UI...

//CẤU HÌNH GAME

int kichThuocO = 25;
// Mỗi ô vuông trên lưới = 25x25 pixels
// Rắn và thức ăn sẽ di chuyển theo từng ô này

int SoOtrongMoiHangCot = 20;
// Tạo lưới 20x20 ô 
// → Tổng kích thước game board = 20 * 25 = 500x500 pixels

int offset = 62.5;
// Khoảng cách đệm từ viền cửa sổ đến game board
// Tạo không gian cho UI (điểm số, title...) xung quanh game board

//BIẾN THỜI GIAN
double lastUpdateTime = 0;
// Lưu timestamp của lần update cuối
// Dùng để kiểm soát tốc độ game (VD: rắn di chuyển mỗi 0.2 giây)
// Tránh rắn chạy quá nhanh theo FPS

//  TÀI NGUYÊN ĐỒ HỌA 
Texture2D backgroundTexture;    // Texture nền game
Texture2D snakeHeadTexture;     // Texture đầu rắn
Texture2D snakeBodyTexture;     // Texture thân rắn
Texture2D foodTextures[5];      // Mảng 5 loại texture thức ăn
int totalFoodTypes = 5;         // Tổng số loại thức ăn

//  ÂM THANH 
Music backgroundMusic;
// File nhạc nền cho menu hoặc gameplay

bool musicPlaying = false;
// Cờ kiểm tra nhạc có đang phát hay không
// Tránh phát nhạc nhiều lần cùng lúc

// HÀM KIỂM TRA PHẦN TỬ TRONG DEQUE 
// Công dụng: Kiểm tra xem một Vector2 có tồn tại trong deque hay không
// Dùng để: Kiểm tra va chạm với thân rắn, kiểm tra vị trí thức ăn

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
     // INPUT: element - tọa độ cần kiểm tra (x, y)
    //        deque - danh sách các tọa độ (VD: thân rắn)
    // OUTPUT: true nếu tìm thấy, false nếu không
    
    // Duyệt qua tất cả phần tử trong deque
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        // So sánh từng phần tử với element cần tìm
        if (Vector2Equals(deque[i], element))
        {
            // Vector2Equals() - hàm của raymath.h
        // So sánh 2 vector có bằng nhau không (cả x và y)
            return true;  // Tìm thấy
        }
    }
    return false;  // Không tìm thấy
}

// ==================== HÀM TRIGGER SỰ KIỆN THEO THỜI GIAN ====================
// Công dụng: Kiểm soát tốc độ update game (game loop timing)
// interval: khoảng thời gian giữa các lần update (giây)
bool eventTriggered(double interval)
{
    // Mục đích: Kiểm soát tốc độ game độc lập với FPS
    // VD: interval = 0.2 → rắn di chuyển 5 lần/giây (không phụ thuộc FPS là 60 hay 144)
    
    double currentTime = GetTime();  // Lấy thời gian hiện tại
    // GetTime() của Raylib - trả về số giây kể từ khi InitWindow()

    // Kiểm tra xem đã đủ thời gian interval chưa
    if (currentTime - lastUpdateTime >= interval)
    {
        // Đã đủ khoảng thời gian interval
        lastUpdateTime = currentTime;  // Cập nhật thời điểm update
        return true;   // Đã đến lúc update
    }
    return false;  // Chưa đến lúc update
}

// ==================== CLASS SNAKE (RẮN) ====================
class Snake
{
public:
    // Body: deque chứa tọa độ các đoạn thân rắn
    // Đầu rắn ở vị trí [0], đuôi ở vị trí cuối
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    // Khởi tạo rắn dài 3 đoạn
    // body[0] = {6,9} - ĐẦU (phải nhất)
    // body[1] = {5,9} - THÂN 
    // body[2] = {4,9} - ĐUÔI (trái nhất)
    // → Rắn đang nằm ngang, hướng sang phải
    
    
    Vector2 direction = {1, 0};
    // (1,0) = di chuyển 1 ô sang phải mỗi lần
    // (0,1) = xuống dưới, (-1,0) = sang trái, (0,-1) = lên trên

    bool addSegment = false;
    // Cờ báo hiệu: true = rắn vừa ăn thức ăn, cần thêm 1 đoạn thân

    // ========== VẼ RẮN ==========
    void Draw()
    {
        // Duyệt qua từng đoạn thân
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;  // Tọa độ x (grid)
            float y = body[i].y;  // Tọa độ y (grid)
            
            // Chuyển đổi tọa độ grid sang pixels
            Rectangle segment = Rectangle{
                offset + x * kichThuocO,// VD: 62.5 + 6*25 = 212.5 pixels
                offset + y * kichThuocO,// VD: 62.5 + 9*25 = 287.5 pixels
                (float)kichThuocO, // Rộng 25px
                (float)kichThuocO // Cao 25px
            };

            if (i == 0) // Vẽ đầu rắn
            {
                // Tính góc xoay dựa vào hướng di chuyển
                float rotation = 0.0f;
                if (direction.x == 1)       // Sang phải
                    rotation = 0.0f;
                else if (direction.x == -1) // Sang trái
                    rotation = 180.0f;
                else if (direction.y == -1) // Lên trên
                    rotation = 270.0f;
                else if (direction.y == 1)  // Xuống dưới
                    rotation = 90.0f;

                // Origin = điểm neo để xoay (tâm texture)
                Vector2 origin = {kichThuocO / 2.0f, kichThuocO / 2.0f};
                // Vị trí tâm của đầu rắn
                Vector2 position = {segment.x + kichThuocO / 2.0f, 
                                   segment.y + kichThuocO / 2.0f};
                
                 // DrawTexturePro: vẽ texture với scale + rotation
                DrawTexturePro(snakeHeadTexture,// Source: lấy toàn bộ texture gốc
                    Rectangle{0, 0, (float)snakeHeadTexture.width, 
                             (float)snakeHeadTexture.height}, 
                              // Destination: vẽ vào vị trí segment 25x25
                    Rectangle{position.x, position.y, 
                             (float)kichThuocO, (float)kichThuocO}, // Dest rect
                    origin,// Xoay quanh tâm
                     rotation, // Góc xoay
                     WHITE // Tint màu (WHITE = giữ nguyên màu gốc)
                    );
                    
            }
            else // Vẽ thân rắn
            {
                float rotation = 0.0f;
                Vector2 currentDirection = {0, 0};
                
                // Tính hướng dựa vào đoạn trước đó
                Vector2 prevSegment = body[i - 1];      // Đoạn phía trước
                Vector2 currentSegment = body[i];       // Đoạn hiện tại
                currentDirection.x = prevSegment.x - currentSegment.x;
                currentDirection.y = prevSegment.y - currentSegment.y;
                // *** VÍ DỤ MINH HỌA ***
    // Giả sử rắn đang nằm ngang:
    // body[0] = {7, 9} (đầu)
    // body[1] = {6, 9} (thân 1) ← đang vẽ đoạn này
    // body[2] = {5, 9} (thân 2)
    //
    // Khi vẽ body[1]:
    // prevSegment = body[0] = {7, 9}
    // currentSegment = body[1] = {6, 9}
    // currentDirection.x = 7 - 6 = 1    → hướng PHẢI
    // currentDirection.y = 9 - 9 = 0
    // → Vector {1, 0} nghĩa là đoạn thân này hướng từ trái sang phải

                // Xác định góc xoay TEXTURE
                // Texture gốc được thiết kế hướng PHẢI (0 độ)
    // Cần xoay để khớp với hướng thực tế
                if (currentDirection.x == 1)// Vector {1, 0} = hướng PHẢI
                    rotation = 0.0f;// Giữ nguyên (0 độ)

                else if (currentDirection.x == -1)// Vector {-1, 0} = hướng TRÁI
                    rotation = 180.0f;// Xoay 180 độ

                else if (currentDirection.y == -1)// Vector {0, -1} = hướng LÊN
                    rotation = 270.0f;// Xoay 270 độ (hay -90 độ)

                else if (currentDirection.y == 1)// Vector {0, 1} = hướng XUỐNG
                    rotation = 90.0f;// Xoay 90 độ

                    // *** HÌNH ẢNH MINH HỌA ***
    //   0° (PHẢI)     90° (XUỐNG)    180° (TRÁI)    270° (LÊN)
    //      →               ↓               ←              ↑
    //    ═══►            ║              ◄═══            ▲
    //                    ▼                              ║
    // THIẾT LẬP VẼ TEXTURE 

                Vector2 origin = {kichThuocO / 2.0f, kichThuocO / 2.0f};
                // Origin = điểm neo để xoay (tâm của texture)
    // VD: origin = {12.5, 12.5} với kichThuocO = 25
    // → Xoay quanh điểm chính giữa, không bị lệch
                Vector2 position = {segment.x + kichThuocO / 2.0f, 
                                   segment.y + kichThuocO / 2.0f};

                                   // Position = tâm của ô cần vẽ (tọa độ pixel)
    // segment.x, segment.y là góc TRÊN-TRÁI của ô
    // Cộng thêm kichThuocO/2 để lấy ĐIỂM GIỮA ô
    
    // *** VÍ DỤ TÍNH TOÁN ***
    // Nếu segment = {212.5, 287.5, 25, 25} (x, y, width, height)
    // → position = {212.5 + 12.5, 287.5 + 12.5} = {225, 300}
    //    (tâm của ô vuông 25x25)
                
                // Vẽ texture thân rắn
                DrawTexturePro(snakeBodyTexture,
                // Texture nguồn (ảnh thân rắn)
        // Source Rectangle: vùng cắt từ texture gốc
                    Rectangle{0, 0, (float)snakeBodyTexture.width,
                         // Lấy toàn bộ chiều rộng
                             (float)snakeBodyTexture.height},// Lấy toàn bộ chiều cao
                             // Destination Rectangle: vị trí + kích thước vẽ lên màn hìn
                    Rectangle{position.x,// Tâm x (đã tính ở trên)
                         position.y, // Tâm y
                             (float)kichThuocO,// Rộng 25 pixels
                              (float)kichThuocO// Cao 25 pixels
                            },
                    origin,// Điểm neo xoay (tâm texture)
                     rotation, // Góc xoay (0, 90, 180, hoặc 270 độ)
                     WHITE); // Tint màu (WHITE = không đổi màu)
            }
        }
    }

    // ========== CẬP NHẬT VỊ TRÍ RẮN ==========
    void Update()
    {
        // Thêm đầu mới = vị trí đầu cũ + hướng di chuyển
        body.push_front(Vector2Add(body[0], direction));
        // - body[0]: vị trí đầu rắn hiện tại
    // - direction: vector hướng di chuyển
    // - Vector2Add(): cộng 2 vector → vị trí đầu mới
    // - push_front(): thêm vào ĐẦU deque
        
        if (!addSegment)
        {
            // Trường hợp: KHÔNG ăn thức ăn
        // → Giữ nguyên chiều dài rắn

            body.pop_back();
            // pop_back(): xóa phần tử CUỐI deque (đuôi rắn)
        
        // Kết quả:
        // body = [{7,9}, {6,9}, {5,9}]  (3 đoạn - giữ nguyên)
        //                        ^^^^ đuôi cũ bị xóa
        
        // → Hiệu ứng: rắn "TRƯỜN" - đầu tiến 1 ô, đuôi theo kịp
        }
        // Reset cờ thêm đoạn

        addSegment = false;
         // Sau mỗi lần Update, luôn reset cờ về false
    // → Chỉ thêm đoạn 1 lần duy nhất khi ăn thức ăn
    }

    // ========== RESET RẮN VỀ TRẠNG THÁI BAN ĐẦU ==========
    void Reset()
    {
        // *** MỤC ĐÍCH ***
    // Đưa rắn về trạng thái khởi tạo ban đầu
    // Dùng khi: Game Over, Restart, hoặc Start New Game
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        // Khôi phục thân rắn về 3 đoạn ban đầu
    // Vị trí: giữa-trái màn hình (trên lưới 20x20)
        direction = {1, 0};// Khôi phục hướng ban đầu = PHẢI
    // Vector {1, 0} nghĩa là: +1 ô theo x, 0 ô theo y
    //
    // Các hướng khác:
    // {-1, 0} = TRÁI
    // {0, 1}  = XUỐNG
    // {0, -1} = LÊN
    }
};

// ==================== CLASS FOOD (THỨC ĂN) ====================
class Food
{
public:
    Vector2 position;   // Vị trí thức ăn (grid coordinates)
    int foodType;       // Loại thức ăn (0-4)

    // Constructor: Tạo thức ăn ở vị trí ngẫu nhiên
    Food(deque<Vector2> snakeBody)
    {
        position = GenerateRandomPos(snakeBody);
        foodType = GetRandomValue(0, totalFoodTypes - 1);
    }

    // ========== VẼ THỨC ĂN ==========
    void Draw()
    {
        // Tính tỷ lệ scale (texture gốc 28x28)
        float scale = (float)kichThuocO / 28.0f;
        
        // Chuyển đổi tọa độ grid sang pixels
        Vector2 pos = {offset + position.x * kichThuocO, 
                      offset + position.y * kichThuocO};
        
        // Vẽ texture thức ăn
        DrawTextureEx(foodTextures[foodType], pos, 0.0f, scale, WHITE);
    }

    // ========== TẠO TỌA ĐỘ NGẪU NHIÊN ==========
    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, SoOtrongMoiHangCot - 1);
        float y = GetRandomValue(0, SoOtrongMoiHangCot - 1);
        return Vector2{x, y};
    }

    // ========== TẠO VỊ TRÍ NGẪU NHIÊN (KHÔNG TRÙNG RẮN) ==========
    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        
        // Lặp lại nếu vị trí trùng với thân rắn
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }

    // ========== TẠO LẠI THỨC ĂN MỚI ==========
    void Regenerate(deque<Vector2> snakeBody)
    {
        position = GenerateRandomPos(snakeBody);
        foodType = GetRandomValue(0, totalFoodTypes - 1);
    }
};

// ==================== CLASS GAME (LOGIC GAME) ====================
class Game
{
public:
    Snake snake = Snake();              // Đối tượng rắn
    Food food = Food(snake.body);       // Đối tượng thức ăn
    bool running = true;                // Trạng thái game đang chạy
    int score = 0;                      // Điểm số
    Sound eatSound;                     // Âm thanh ăn
    Sound wallSound;                    // Âm thanh va tường
    bool isDead = false;                // Trạng thái chết

    // ========== CONSTRUCTOR: KHỞI TẠO ÂM THANH ==========
    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    // ========== DESTRUCTOR: GIẢI PHÓNG ÂM THANH ==========
    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    // ========== VẼ GAME ==========
    void Draw()
    {
        food.Draw();   // Vẽ thức ăn
        snake.Draw();  // Vẽ rắn
    }

    // ========== CẬP NHẬT GAME ==========
    void Update()
    {
        if (running)
        {
            snake.Update();              // Di chuyển rắn
            CheckCollisionWithFood();    // Kiểm tra ăn thức ăn
            CheckCollisionWithEdges();   // Kiểm tra va tường
            CheckCollisionWithTail();    // Kiểm tra va thân
        }
    }

    // ========== KIỂM TRA VA CHẠM VỚI THỨC ĂN ==========
    void CheckCollisionWithFood()
    {
        // So sánh vị trí đầu rắn với vị trí thức ăn
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.Regenerate(snake.body);  // Tạo thức ăn mới
            snake.addSegment = true;      // Đánh dấu cần thêm đoạn
            score++;                      // Tăng điểm
            PlaySound(eatSound);          // Phát âm thanh
        }
    }

    // ========== KIỂM TRA VA CHẠM VỚI TƯỜNG ==========
    void CheckCollisionWithEdges()
    {
        // Kiểm tra đầu rắn có ra ngoài biên không
        if (snake.body[0].x == SoOtrongMoiHangCot || snake.body[0].x == -1 ||
            snake.body[0].y == SoOtrongMoiHangCot || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    // ========== XỬ LÝ GAME OVER ==========
    void GameOver()
    {
        snake.Reset();                   // Reset rắn
        food.Regenerate(snake.body);     // Tạo thức ăn mới
        running = false;                 // Dừng game
        isDead = true;                   // Đánh dấu đã chết
        PlaySound(wallSound);            // Phát âm thanh
    }

    // ========== KIỂM TRA VA CHẠM VỚI THÂN ==========
    void CheckCollisionWithTail()
    {
        // Tạo bản sao thân rắn không có đầu
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        
        // Kiểm tra đầu có trùng với thân không
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    // ========== KHỞI ĐỘNG LẠI GAME ==========
    void Restart()
    {
        snake.Reset();
        food.Regenerate(snake.body);
        score = 0;
        isDead = false;
        running = true;
    }
};

// ==================== ENUM MÀN HÌNH GAME ====================
enum GameScreen
{
    MENU,   // Màn hình menu
    GAME    // Màn hình game
};

// ==================== HÀM MAIN ====================
int main()
{
    GameScreen currentScreen = MENU;  // Bắt đầu ở menu
    cout << "Starting the game..." << endl;

    // Tính kích thước cửa sổ
    int windowWidth = 2 * offset + kichThuocO * SoOtrongMoiHangCot;
    int windowHeight = 2 * offset + kichThuocO * SoOtrongMoiHangCot;

    // Khởi tạo cửa sổ game
    InitWindow(windowWidth, windowHeight, "Snake Game");
    SetTargetFPS(60);  // Giới hạn 60 FPS

    // ========== LOAD TÀI NGUYÊN MENU ==========
    Texture2D menubackground = LoadTexture("Graphics/background.png");
    Button startButton{"Graphics/start.png", {250, 420}, 3.5};
    Button exitButton{"Graphics/exit.png", {250, 480}, 3.5};

    // ========== LOAD TÀI NGUYÊN GAME ==========
    Image backgroundImage = LoadImage("Graphics/san.png");
    backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);

    Image headImage = LoadImage("Graphics/dau.png");
    snakeHeadTexture = LoadTextureFromImage(headImage);
    UnloadImage(headImage);

    Image bodyImage = LoadImage("Graphics/than.png");
    snakeBodyTexture = LoadTextureFromImage(bodyImage);
    UnloadImage(bodyImage);

    // Load 5 loại thức ăn
    for (int i = 0; i < totalFoodTypes; i++)
    {
        string foodPath = "Graphics/Food" + to_string(i + 1) + ".png";
        Image foodImage = LoadImage(foodPath.c_str());
        foodTextures[i] = LoadTextureFromImage(foodImage);
        UnloadImage(foodImage);
    }

    Game game = Game();  // Khởi tạo game

    // Load nhạc nền menu
    backgroundMusic = LoadMusicStream("Sounds/nhacnen.mp3");

    // ==================== GAME LOOP CHÍNH ====================
    while (!WindowShouldClose())
    {
        // Lấy input chuột
        Vector2 mousePosition = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        // ========== XỬ LÝ NHẠC NỀN ==========
        if (currentScreen == MENU)
        {
            // Phát nhạc ở menu
            if (!musicPlaying)
            {
                PlayMusicStream(backgroundMusic);
                musicPlaying = true;
            }
            UpdateMusicStream(backgroundMusic);
        }
        else if (currentScreen == GAME)
        {
            // Tắt nhạc khi vào game
            if (musicPlaying)
            {
                StopMusicStream(backgroundMusic);
                musicPlaying = false;
            }
        }

        BeginDrawing();

        // ==================== VẼ MÀN HÌNH MENU ====================
        if (currentScreen == MENU)
        {
            ClearBackground(green);

            // Scale background để fit cửa sổ
            float scaleX = (float)windowWidth / menubackground.width;
            float scaleY = (float)windowHeight / menubackground.height;
            float scale = fmaxf(scaleX, scaleY);
            DrawTextureEx(menubackground, Vector2{0, 0}, 0.0f, scale, WHITE);

            // Vẽ các nút
            startButton.Draw();
            exitButton.Draw();

            // Xử lý click nút Start
            if (startButton.isPressed(mousePosition, mousePressed))
            {
                game.Restart();
                currentScreen = GAME;
            }

            // Xử lý click nút Exit
            if (exitButton.isPressed(mousePosition, mousePressed))
            {
                break;  // Thoát game loop
            }
        }
        // ==================== VẼ MÀN HÌNH GAME ====================
        else if (currentScreen == GAME)
        {
            // Update game mỗi 0.2 giây
            if (eventTriggered(0.2))
            {
                game.Update();
            }

            // Xử lý input điều khiển rắn
            if (game.running)
            {
                // Không cho đi ngược lại
                if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1)
                    game.snake.direction = {0, -1};
                if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1)
                    game.snake.direction = {0, 1};
                if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1)
                    game.snake.direction = {-1, 0};
                if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1)
                    game.snake.direction = {1, 0};
            }

            // Nhấn Q để quay về menu
            if (IsKeyPressed(KEY_Q))
            {
                currentScreen = MENU;
                game.running = false;
                game.isDead = false;
            }

            // Nhấn R để chơi lại khi chết
            if (game.isDead && IsKeyPressed(KEY_R))
            {
                game.Restart();
            }

            // Vẽ background
            float scaleX = (float)windowWidth / backgroundTexture.width;
            float scaleY = (float)windowHeight / backgroundTexture.height;
            float scale = fmaxf(scaleX, scaleY);
            DrawTextureEx(backgroundTexture, Vector2{0, 0}, 0.0f, scale, WHITE);

            // Vẽ viền game board
            DrawRectangleLinesEx(
                Rectangle{(float)offset - 5, (float)offset - 5, 
                         (float)SoOtrongMoiHangCot * kichThuocO + 10, 
                         (float)SoOtrongMoiHangCot * kichThuocO + 10}, 
                5, darkgreen);
            
            // Vẽ tiêu đề
            DrawText("Snake Game", offset - 5, 20, 40, darkgreen);
            
            // Vẽ điểm số
            DrawText(TextFormat("Score: %i", game.score), 
                    offset - 5, offset + 5 + SoOtrongMoiHangCot * kichThuocO, 
                    40, darkgreen);

            // Vẽ game (rắn + thức ăn)
            game.Draw();

            // ========== VẼ OVERLAY KHI CHẾT ==========
            if (game.isDead)
            {
                // Làm tối màn hình
                DrawRectangle(0, 0, windowWidth, windowHeight, Fade(BLACK, 0.7f));
                
                // Hiển thị thông báo
                DrawText("GAME OVER!", windowWidth / 2 - 120, 
                        windowHeight / 2 - 60, 40, RED);
                DrawText(TextFormat("Final Score: %i", game.score), 
                        windowWidth / 2 - 80, windowHeight / 2 - 10, 20, WHITE);
                DrawText("Press R to Restart", windowWidth / 2 - 90, 
                        windowHeight / 2 + 20, 20, WHITE);
                DrawText("Press Q to Menu", windowWidth / 2 - 85, 
                        windowHeight / 2 + 50, 20, WHITE);
            }
            // ========== HƯỚNG DẪN KHI CHƯA BẮT ĐẦU ==========
            else if (!game.running && !game.isDead)
            {
                DrawText("Press Arrow Keys to Start", 
                        windowWidth / 2 - 120, windowHeight / 2, 20, darkgreen);
                DrawText("Q - Back to Menu", windowWidth / 2 - 80, 
                        windowHeight / 2 + 30, 16, darkgreen);
            }
        }

        EndDrawing();
    }

    // ==================== GIẢI PHÓNG TÀI NGUYÊN ====================
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