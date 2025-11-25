#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <string>
#include "button.hpp"
#include <vector>

using namespace std;

// ============ CÀI ĐẶT MÀU SẮC ============
Color mauXanhLa = {173, 204, 96, 255};      
Color mauXanhDam = {43, 51, 24, 255};       

// ============ CÀI ĐẶT GAME ============
int kichThuocO = 50;                        
int soOTrongMoiHangCot = 20;                
int leManhGame = 62.5;                      
float tiLeDauRan  = 1.0f;   // chỉnh kích thước đầu rắn
float tiLeThanRan = 1.0f;   // chỉnh kích thước thân rắn
float tiLeThucAn  = 0.8f;   // chỉnh kích thước thức ăn
double thoiGianCapNhatCuoi = 0;             

// ============ TEXTURE VÀ HÌNH ẢNH ============
Texture2D hinhNenGame;                      
Texture2D hinhDauRan;                       
Texture2D hinhThanRan;                      
Texture2D hinhThucAn[5];                    
int tongSoLoaiThucAn = 5;                   

// ============ ÂM THANH VÀ NHẠC NỀN ============
Music nhacNenMenu;                          
bool dangPhatNhac = false;                  

bool kiemTraPhanTuTrongDeque(Vector2 phanTu, deque<Vector2> danhSach)
{
    for (unsigned int i = 0; i < danhSach.size(); i++)
    {
        if (Vector2Equals(danhSach[i], phanTu))
        {
            return true;
        }
    }
    return false;
}

bool kiemTraSuKienThoiGian(double khoangThoiGian)
{
    double thoiGianHienTai = GetTime();
    if (thoiGianHienTai - thoiGianCapNhatCuoi >= khoangThoiGian)
    {
        thoiGianCapNhatCuoi = thoiGianHienTai;
        return true;
    }
    return false;
}

class ConRan
{
public:
    deque<Vector2> thanRan = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};  
    Vector2 huongDiChuyen = {1, 0};                                          
    bool themDoanMoi = false;                                                

    void veRan()
    {
        for (unsigned int i = 0; i < thanRan.size(); i++)
        {
            float x = thanRan[i].x;
            float y = thanRan[i].y;

            // Tâm ô lưới
             float centerX = leManhGame + (x + 0.5f) * kichThuocO;
             float centerY = leManhGame + (y + 0.5f) * kichThuocO;


            if (i == 0) 
{
    float gocXoay = 0.0f;
    if (huongDiChuyen.x == 1)      gocXoay = 0.0f;
    else if (huongDiChuyen.x == -1) gocXoay = 180.0f;
    else if (huongDiChuyen.y == -1) gocXoay = 270.0f;
    else if (huongDiChuyen.y == 1)  gocXoay = 90.0f;

    // kích thước đầu rắn tính theo tỉ lệ riêng
    float width  = kichThuocO * tiLeDauRan;
    float height = kichThuocO * tiLeDauRan;

    Rectangle dest = {centerX, centerY, width, height};
    Vector2 diemGoc = {width / 2.0f, height / 2.0f};

    DrawTexturePro(
        hinhDauRan,
        Rectangle{0, 0, (float)hinhDauRan.width, (float)hinhDauRan.height},
        dest,
        diemGoc,
        gocXoay,
        WHITE
    );
}
            else 
{
    float gocXoay = 0.0f;
    Vector2 huongHienTai = {0, 0};
    Vector2 doanTruoc = thanRan[i - 1];
    Vector2 doanHienTai = thanRan[i];
    huongHienTai.x = doanTruoc.x - doanHienTai.x;
    huongHienTai.y = doanTruoc.y - doanHienTai.y;

    if (huongHienTai.x == 1)       gocXoay = 0.0f;
    else if (huongHienTai.x == -1) gocXoay = 180.0f;
    else if (huongHienTai.y == -1) gocXoay = 270.0f;
    else if (huongHienTai.y == 1)  gocXoay = 90.0f;

    // kích thước thân rắn tính theo tỉ lệ riêng
    float width  = kichThuocO * tiLeThanRan;
    float height = kichThuocO * tiLeThanRan;

    Rectangle dest = {centerX, centerY, width, height};
    Vector2 diemGoc = {width / 2.0f, height / 2.0f};

    DrawTexturePro(
        hinhThanRan,
        Rectangle{0, 0, (float)hinhThanRan.width, (float)hinhThanRan.height},
        dest,
        diemGoc,
        gocXoay,
        WHITE
    );
}

        }
    }

    void capNhatRan()
    {
        thanRan.push_front(Vector2Add(thanRan[0], huongDiChuyen));
        if (!themDoanMoi)
        {
            thanRan.pop_back();
        }
        themDoanMoi = false; 
    }

    void datLaiRan()
    {
        thanRan = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        huongDiChuyen = {1, 0};
    }
};

class ThucAn
{
public:
    Vector2 viTri;          
    int loaiThucAn;         

    ThucAn(deque<Vector2> thanRan)
    {
        viTri = taoViTriNgauNhien(thanRan);
        loaiThucAn = GetRandomValue(0, tongSoLoaiThucAn - 1);
    }

    void veThucAn()
    {
        // scale cơ bản: scale texture gốc vừa khít 1 ô
    float baseScale = (float)kichThuocO / 28.0f;  // 28 là size gốc texture của bạn
    float scale = baseScale * tiLeThucAn;         // nhân tỉ lệ riêng thức ăn

    // kích thước thức ăn sau khi scale
    float w = hinhThucAn[loaiThucAn].width  * scale;
    float h = hinhThucAn[loaiThucAn].height * scale;

    // canh giữa trong ô
    float pixelX = leManhGame + viTri.x * kichThuocO + (kichThuocO - w) / 2.0f;
    float pixelY = leManhGame + viTri.y * kichThuocO + (kichThuocO - h) / 2.0f;

    Vector2 viTriThucTe = {pixelX, pixelY};
    DrawTextureEx(hinhThucAn[loaiThucAn], viTriThucTe, 0.0f, scale, WHITE);
    }

    Vector2 taoONgauNhien()
    {
        float x = GetRandomValue(0, soOTrongMoiHangCot - 1);
        float y = GetRandomValue(0, soOTrongMoiHangCot - 1);
        return Vector2{x, y};
    }

    Vector2 taoViTriNgauNhien(deque<Vector2> thanRan)
    {
        Vector2 viTriMoi = taoONgauNhien();
        while (kiemTraPhanTuTrongDeque(viTriMoi, thanRan))
        {
            viTriMoi = taoONgauNhien();
        }
        return viTriMoi;
    }

    void taoLaiThucAn(deque<Vector2> thanRan)
    {
        viTri = taoViTriNgauNhien(thanRan);
        loaiThucAn = GetRandomValue(0, tongSoLoaiThucAn - 1);
    }
};
const int SO_LUONG_THUC_AN = 2;   // muốn 2 mồi
bool tranhTrungThucAn = true;
class TroChoiRanSanMoi
{
public:
    ConRan ran = ConRan();                  
    std::vector<ThucAn> dsThucAn;   // danh sách nhiều mồi  
    bool dangChay = true;                   
    int diem = 0;                          
    Sound amThanhAn;                       
    Sound amThanhVaCham;                   
    bool daGhet = false;                   

    TroChoiRanSanMoi()
    {
        InitAudioDevice();
        amThanhAn = LoadSound("Sounds/eat.mp3");
        amThanhVaCham = LoadSound("Sounds/wall.mp3");
        taoTatCaThucAn();
    }

    ~TroChoiRanSanMoi()
    {
        UnloadSound(amThanhAn);
        UnloadSound(amThanhVaCham);
        CloseAudioDevice();
    }

    void veGame()
    {
        for (auto &food : dsThucAn)
        food.veThucAn();

        ran.veRan();
    }

    void capNhatGame()
    {
        if (dangChay)
        {
            ran.capNhatRan();
            kiemTraVaChamVoiThucAn();
            kiemTraVaChamVoiTuong();
            kiemTraVaChamVoiThan();
        }
    }

    void kiemTraVaChamVoiThucAn()
    {
        // duyệt qua từng mồi
        for (int i = 0; i < (int)dsThucAn.size(); ++i)
        {
            auto &food = dsThucAn[i];
            if (Vector2Equals(ran.thanRan[0], food.viTri))
            {
                taoLaiThucAnKhongTrung(i);  // random lại mồi này, tránh trùng mồi khác
                ran.themDoanMoi = true;
                diem++;
                PlaySound(amThanhAn);
            }
        }
        
    }

    void kiemTraVaChamVoiTuong()
    {
        if (ran.thanRan[0].x == soOTrongMoiHangCot || ran.thanRan[0].x == -1 ||
            ran.thanRan[0].y == soOTrongMoiHangCot || ran.thanRan[0].y == -1)
        {
            ketThucGame();
        }
    }

    void ketThucGame()
    {
        ran.datLaiRan();
        taoTatCaThucAn();
        dangChay = false;
        daGhet = true;
        PlaySound(amThanhVaCham);
    }

    void kiemTraVaChamVoiThan()
    {
        deque<Vector2> thanKhongCoDau = ran.thanRan;
        thanKhongCoDau.pop_front(); 
        
        if (kiemTraPhanTuTrongDeque(ran.thanRan[0], thanKhongCoDau))
        {
            ketThucGame();
        }
    }

    void khoiDongLai()
    {
        ran.datLaiRan();
        for (auto &food : dsThucAn)
        taoTatCaThucAn();
        diem = 0;
        daGhet = false;
        dangChay = true;
    }
};

enum ManHinhGame
{
    MENU,    
    GAME     
};

int main()
{
    ManHinhGame manHinhHienTai = MENU;
    cout << "Đang khởi động game..." << endl;

    int chieuRongCuaSo = 2 * leManhGame + kichThuocO * soOTrongMoiHangCot;
    int chieuCaoCuaSo = 2 * leManhGame + kichThuocO * soOTrongMoiHangCot;

    InitWindow(chieuRongCuaSo, chieuCaoCuaSo, "Game Rắn Săn Mồi");
    SetTargetFPS(60);

    Texture2D hinhNenMenu = LoadTexture("Graphics/background.png");
    Button nutStart{"Graphics/start.png", {0, 0}, 5};
    Button nutThoat{"Graphics/exit.png", {0, 0}, 5};
    nutStart.setPosition({
    (chieuRongCuaSo - nutStart.getTexture().width) / 2.0f,
    chieuCaoCuaSo * 0.60f
     });
      nutThoat.setPosition({
    (chieuRongCuaSo - nutThoat.getTexture().width) / 2.0f,
    chieuCaoCuaSo * 0.70f
      });

    Image anhNenGame = LoadImage("Graphics/san.png");
    hinhNenGame = LoadTextureFromImage(anhNenGame);
    UnloadImage(anhNenGame);

    Image anhDauRan = LoadImage("Graphics/dau2.png");
    hinhDauRan = LoadTextureFromImage(anhDauRan);
    UnloadImage(anhDauRan);

    Image anhThanRan = LoadImage("Graphics/than.png");
    hinhThanRan = LoadTextureFromImage(anhThanRan);
    UnloadImage(anhThanRan);

    for (int i = 0; i < tongSoLoaiThucAn; i++)
    {
        string duongDanThucAn = "Graphics/Food" + to_string(i + 1) + ".png";
        Image anhThucAn = LoadImage(duongDanThucAn.c_str());
        hinhThucAn[i] = LoadTextureFromImage(anhThucAn);
        UnloadImage(anhThucAn);
    }

    TroChoiRanSanMoi game = TroChoiRanSanMoi();
    nhacNenMenu = LoadMusicStream("Sounds/nhacnen.mp3");

    while (!WindowShouldClose()) 
    {
        Vector2 viTriChuot = GetMousePosition();
        bool nhanChuot = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (manHinhHienTai == MENU)
        {
            if (!dangPhatNhac)
            {
                PlayMusicStream(nhacNenMenu);
                dangPhatNhac = true;
            }
            UpdateMusicStream(nhacNenMenu); 
        }
        else if (manHinhHienTai == GAME)
        {
            if (dangPhatNhac)
            {
                StopMusicStream(nhacNenMenu);
                dangPhatNhac = false;
            }
        }

        BeginDrawing(); 

        if (manHinhHienTai == MENU)
        {
            ClearBackground(mauXanhLa);

            float tyLeX = (float)chieuRongCuaSo / hinhNenMenu.width;
            float tyLeY = (float)chieuCaoCuaSo / hinhNenMenu.height;
            float tyLe = fmaxf(tyLeX, tyLeY); 
            DrawTextureEx(hinhNenMenu, Vector2{0, 0}, 0.0f, tyLe, WHITE);

            nutStart.Draw();
            nutThoat.Draw();

            if (nutStart.isPressed(viTriChuot, nhanChuot))
            {
                game.khoiDongLai();
                manHinhHienTai = GAME;
            }

            if (nutThoat.isPressed(viTriChuot, nhanChuot))
            {
                break; 
            }
        }
        else if (manHinhHienTai == GAME)
        {
            if (kiemTraSuKienThoiGian(0.2))
            {
                game.capNhatGame();
            }

            if (game.dangChay) 
            {
                if (IsKeyPressed(KEY_UP) && game.ran.huongDiChuyen.y != 1)
                    game.ran.huongDiChuyen = {0, -1};
                if (IsKeyPressed(KEY_DOWN) && game.ran.huongDiChuyen.y != -1)
                    game.ran.huongDiChuyen = {0, 1};
                if (IsKeyPressed(KEY_LEFT) && game.ran.huongDiChuyen.x != 1)
                    game.ran.huongDiChuyen = {-1, 0};
                if (IsKeyPressed(KEY_RIGHT) && game.ran.huongDiChuyen.x != -1)
                    game.ran.huongDiChuyen = {1, 0};
            }

            if (IsKeyPressed(KEY_Q))
            {
                manHinhHienTai = MENU;
                game.dangChay = false;
                game.daGhet = false;
            }

            if (game.daGhet && IsKeyPressed(KEY_R))
            {
                game.khoiDongLai();
            }

            float tyLeX = (float)chieuRongCuaSo / hinhNenGame.width;
            float tyLeY = (float)chieuCaoCuaSo / hinhNenGame.height;
            float tyLe = fmaxf(tyLeX, tyLeY);
            DrawTextureEx(hinhNenGame, Vector2{0, 0}, 0.0f, tyLe, WHITE);

            DrawRectangleLinesEx(Rectangle{(float)leManhGame - 5, (float)leManhGame - 5, (float)soOTrongMoiHangCot * kichThuocO + 10, (float)soOTrongMoiHangCot * kichThuocO + 10}, 5, mauXanhDam);

            DrawText("Snake Game", leManhGame - 5, 20, 40, mauXanhDam);
            DrawText(TextFormat("Score: %i", game.diem), leManhGame - 5, leManhGame + 5 + soOTrongMoiHangCot * kichThuocO, 40, mauXanhDam);

            game.veGame();

            if (game.daGhet)
            {
                DrawRectangle(0, 0, chieuRongCuaSo, chieuCaoCuaSo, Fade(BLACK, 0.7f));
                DrawText("GAME OVER!", chieuRongCuaSo / 2 - 120, chieuCaoCuaSo / 2 - 60, 40, RED);
                DrawText(TextFormat("Your Score: %i", game.diem), chieuRongCuaSo / 2 - 80, chieuCaoCuaSo / 2 - 10, 20, WHITE);
                DrawText("Press R to replay", chieuRongCuaSo / 2 - 90, chieuCaoCuaSo / 2 + 20, 20, WHITE);
                DrawText("Press Q to return to menu", chieuRongCuaSo / 2 - 85, chieuCaoCuaSo / 2 + 50, 20, WHITE);
            }
            else if (!game.dangChay && !game.daGhet)
            {
                DrawText("Press the arrow keys to start", chieuRongCuaSo / 2 - 120, chieuCaoCuaSo / 2, 20, mauXanhDam);
                DrawText("Q- Return to Menu", chieuRongCuaSo / 2 - 80, chieuCaoCuaSo / 2 + 30, 16, mauXanhDam);
            }
        }

        EndDrawing(); 
    }

    UnloadTexture(hinhNenMenu);
    UnloadTexture(hinhNenGame);
    UnloadTexture(hinhDauRan);
    UnloadTexture(hinhThanRan);
    
    for (int i = 0; i < tongSoLoaiThucAn; i++)
    {
        UnloadTexture(hinhThucAn[i]);
    }

    UnloadMusicStream(nhacNenMenu);
    CloseWindow();
    return 0;
}