#pragma once
#include <raylib.h> 
class Button
{
    public:
        Button(const char* imagePath, Vector2 imagePosition, float scale);
        ~Button();
        void Draw();
        bool isPressed(Vector2 mousePos,bool mousePressed);
        Texture2D getTexture() { return texture; };
    void setPosition(Vector2 pos) { position = pos; };
    private:
          Texture2D texture;
          Vector2 position;
};