#ifndef GAME_H
#define GAME_H

typedef unsigned int uint;

int platformMouseX(void);
int platformMouseY(void);
int platformClicked(void);
int platformKeyDown(char key);
void platformDrawRect(int x, int y, int w, int h, uint color);
void platformDrawCircle(int x, int y, int r, uint color);

void gameInit(int w, int h);
void gameResize(int w, int h);
void gameRender(void);
void gameUpdate(void);

#endif
