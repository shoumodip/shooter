#include "game.h"
#include <ctype.h>
#include <raylib.h>

#define WIDTH 800
#define HEIGHT 600

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

void platformDrawRect(int x, int y, int w, int h, uint color) {
  DrawRectangle(x, y, w, h, colorFromHex(color));
}

void platformDrawCircle(int x, int y, int r, uint color) {
  DrawCircle(x, y, r, colorFromHex(color));
}

int main(void) {
  gameInit(WIDTH, HEIGHT);

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(WIDTH, HEIGHT, "Shooter");
  SetTargetFPS(60);
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
