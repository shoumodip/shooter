#include "game.h"
#include <ctype.h>
#include <raylib.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600
#define FONT_SIZE 30

Font font;

Color colorFromHex(uint color) {
  return (Color){
    (color >> (3 * 8)) & 0xFF,
    (color >> (2 * 8)) & 0xFF,
    (color >> (1 * 8)) & 0xFF,
    (color >> (0 * 8)) & 0xFF,
  };
}

int platformMouseX(void) {
  return GetMouseX();
}

int platformMouseY(void) {
  return GetMouseY();
}

int platformClicked(void) {
  return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

int platformKeyDown(char key) {
  return IsKeyDown(toupper(key));
}

int platformKeyPressed(char key) {
  return IsKeyPressed(toupper(key));
}

void platformDrawRect(int x, int y, int w, int h, uint color) {
  DrawRectangle(x, y, w, h, colorFromHex(color));
}

void platformDrawText(int w, int h, const char *text) {
  Vector2 size = MeasureTextEx(font, text, FONT_SIZE, 0);
  Vector2 start = {(w - size.x) / 2, (h - size.y) / 2};
  DrawTextEx(font, text, start, FONT_SIZE, 0, WHITE);
}

void platformDrawCircle(int x, int y, int r, uint color) {
  DrawCircle(x, y, r, colorFromHex(color));
}

int main(void) {
  gameInit();
  gameResize(WIDTH, HEIGHT);

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(WIDTH, HEIGHT, "Shooter");
  SetTargetFPS(60);

  font = LoadFontEx("fonts/iosevka.ttf", 30, 0, 0);
  while (!WindowShouldClose()) {
    BeginDrawing();
    gameRender();
    EndDrawing();

    gameUpdate();
    if (IsWindowResized()) {
      gameResize(GetScreenWidth(), GetScreenHeight());
    }
  }
  CloseWindow();
}
