#include "game.h"
#include <stdbool.h>

static const char *itoa(char *buf, int n) {
  int size;
  if (n == 0) {
    size = 1;
  } else {
    size = 0;
    for (int i = n; i != 0; i /= 10) {
      size += 1;
    }
  }

  int end = size - 1;
  if (n == 0) {
    buf[end] = '0';
  } else {
    while (n != 0) {
      buf[end] = '0' + n % 10;
      end -= 1;
      n /= 10;
    }
  }

  buf[size] = '\0';
  return buf;
}

static int strlen(const char *src) {
  int n = 0;
  while (src[n] != '\0') {
    n++;
  }
  return n;
}

#define RAND_A 6364136223846793005ULL
#define RAND_C 1442695040888963407ULL

static int rand(int low, int high) {
  static long long rand_state = 0;
  rand_state = rand_state * RAND_A + RAND_C;
  return ((rand_state >> 32) % (high - low + 1)) + low;
}

static float fabsf(float x) {
  if (x < 0.0f) {
    return -x;
  }
  return x;
}

static float sqrtf(float a) {
  float x = a;
  for (long i = 0; i < 1000 && fabsf(x * x - a) > 1e-6; ++i) {
    x -= (x * x - a) / (2 * x);
  }
  return x;
}

#define SCORE_HEIGHT 25
#define SCORE_WIDTH ((int)(SCORE_HEIGHT * 0.5))

#define TITLE_HEIGHT 35
#define TITLE_WIDTH ((int)(TITLE_HEIGHT * 0.5))

#define SHADOW 0x000000AA
#define FOREGROUND 0xD8A657FF
#define BACKGROUND 0x2C2C2CFF

#define ENEMY_SIZE 25
#define ENEMY_COLOR 0xDCDCDCFF
#define ENEMY_SPEED 4

#define BULLET_SIZE 10
#define BULLET_COLOR 0xEA6962FF
#define BULLET_SPEED 12

#define PLAYER_LIFE 3
#define PLAYER_SIZE 25
#define PLAYER_COLOR 0xEA6962FF
#define PLAYER_SPEED 8

#define HEAL_DELAY 1000
#define SHOOT_DELAY 10
#define SPAWN_DELAY 100
#define DESPAWN_DISTANCE 2500

typedef struct {
  float x;
  float y;
} Vec;

Vec vecAdd(Vec a, Vec b) {
  a.x += b.x;
  a.y += b.y;
  return a;
}

Vec vecSub(Vec a, Vec b) {
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

float vecLen(Vec v) {
  return sqrtf(v.x * v.x + v.y * v.y);
}

float vecDist(Vec a, Vec b) {
  return vecLen(vecSub(a, b));
}

Vec vecLimit(Vec v, float s) {
  if (v.x == 0 && v.y == 0) {
    return v;
  }

  s /= vecLen(v);
  v.x *= s;
  v.y *= s;
  return v;
}

typedef struct {
  int left;
  int delay;
} Timer;

int timerReady(Timer *timer) {
  if (timer->left) {
    timer->left--;
    return 0;
  }
  return 1;
}

void timerReset(Timer *timer) {
  timer->left = timer->delay;
}

typedef struct {
  int life;
  Vec position;
  Vec velocity;
} Sprite;

void spriteInit(Sprite *s, Vec position, Vec velocity) {
  s->life = 1;
  s->position = position;
  s->velocity = velocity;
}

void spriteDraw(Sprite *s, Vec camera, Vec screen, int radius, uint color) {
  if (s->life) {
    Vec position = vecAdd(vecSub(s->position, camera), screen);
    platformDrawCircle(position.x, position.y, radius, color);
  }
}

void spriteMove(Sprite *s) {
  if (s->life) {
    s->position = vecAdd(s->position, s->velocity);
  }
}

int spriteAway(Sprite *s, Vec camera, int min) {
  return vecDist(s->position, camera) >= min;
}

int spriteTouch(Sprite *a, Sprite *b, int min) {
  if (a->life && b->life && vecDist(a->position, b->position) < min) {
    a->life--;
    b->life--;
    return 1;
  }
  return 0;
}

#define SPRITE_LIST_CAPACITY 1024

typedef struct {
  Sprite data[SPRITE_LIST_CAPACITY];
  int count;

  int free[SPRITE_LIST_CAPACITY];
  int freed;
} Sprites;

void spritesNew(Sprites *s, Vec position, Vec velocity) {
  if (s->freed) {
    int index = s->free[--s->freed];
    spriteInit(&s->data[index], position, velocity);
  } else if (s->count < SPRITE_LIST_CAPACITY) {
    spriteInit(&s->data[s->count++], position, velocity);
  }
}

void spritesFree(Sprites *s, int index) {
  s->data[index].life = 0;
  s->free[s->freed++] = index;
}

typedef struct {
  int score;
  Vec screen;
  bool paused;
  bool started;
  Sprite player;
  Sprites bullets;
  Sprites enemies;

  Timer heal;
  Timer shoot;
  Timer spawn;
} Game;

Game game;

void gameInit(void) {
  game.score = 0;
  game.paused = !game.started;

  game.player.life = PLAYER_LIFE;
  game.player.position = (Vec){0};
  game.player.velocity = (Vec){0};

  game.bullets.count = 0;
  game.bullets.freed = 0;

  game.enemies.count = 0;
  game.enemies.freed = 0;

  game.heal.left = 0;
  game.heal.delay = HEAL_DELAY;

  game.shoot.left = 0;
  game.shoot.delay = SHOOT_DELAY;

  game.spawn.left = 0;
  game.spawn.delay = SPAWN_DELAY;
}

void gameRender(void) {
  platformDrawRect(0, 0, game.screen.x * 2, game.screen.y * 2, BACKGROUND);

  for (int i = 0; i < game.enemies.count; ++i) {
    spriteDraw(&game.enemies.data[i], game.player.position, game.screen,
               ENEMY_SIZE, ENEMY_COLOR);
  }

  for (int i = 0; i < game.bullets.count; ++i) {
    spriteDraw(&game.bullets.data[i], game.player.position, game.screen,
               BULLET_SIZE, BULLET_COLOR);
  }

  spriteDraw(&game.player, game.player.position, game.screen, PLAYER_SIZE,
             PLAYER_COLOR);

  char score[32];
  platformDrawText(SCORE_WIDTH, SCORE_HEIGHT * 0.4, SCORE_HEIGHT,
                   itoa(score, game.score), FOREGROUND);

  for (int i = 0; i < game.player.life; ++i) {
    platformDrawCircle(BULLET_SIZE * (i * 3 + 2), BULLET_SIZE * 5, BULLET_SIZE,
                       PLAYER_COLOR);
  }

  if (game.paused || !game.player.life) {
    int w = game.screen.x * 2;
    int h = game.screen.y * 2;
    platformDrawRect(0, 0, w, h, SHADOW);

    const char *text = "Paused (Space to play)";
    if (!game.started) {
      text = "Shooter (Space to play)";
    }

    if (!game.player.life) {
      text = "Game Over (Space to restart)";
    }

    int x = (w - TITLE_WIDTH * strlen(text)) / 2;
    int y = h / 2 - TITLE_HEIGHT;
    platformDrawText(x, y, TITLE_HEIGHT, text, FOREGROUND);
  }
}

void gameResize(int w, int h) {
  game.screen = (Vec){w / 2.0, h / 2.0};
}

void gameUpdate(void) {
  if (platformMousePressed() && (game.paused || !game.player.life)) {
    if (game.player.life) {
      game.paused = !game.paused;
      game.started = true;
    } else {
      gameInit();
    }
    return;
  }

  if (platformKeyPressed(' ')) {
    if (game.player.life) {
      game.paused = !game.paused;
      game.started = true;
    } else {
      gameInit();
    }
  }

  if (game.paused || !game.player.life) {
    return;
  }

  game.player.velocity = (Vec){0};

  if (platformKeyDown('w')) {
    game.player.velocity.y -= 1;
  }

  if (platformKeyDown('a')) {
    game.player.velocity.x -= 1;
  }

  if (platformKeyDown('s')) {
    game.player.velocity.y += 1;
  }

  if (platformKeyDown('d')) {
    game.player.velocity.x += 1;
  }

  game.player.velocity = vecLimit(game.player.velocity, PLAYER_SPEED);
  spriteMove(&game.player);

  if (timerReady(&game.heal)) {
    if (game.player.life < PLAYER_LIFE) {
      game.player.life++;
    }
    timerReset(&game.heal);
  }

  if (timerReady(&game.shoot)) {
    if (platformMouseDown()) {
      Vec mouse = {platformMouseX(), platformMouseY()};
      Vec target = vecSub(mouse, game.screen);
      target = vecLimit(target, BULLET_SPEED);
      spritesNew(&game.bullets, game.player.position, target);
      timerReset(&game.shoot);
    }
  }

  if (timerReady(&game.spawn)) {
    int side = rand(0, 3);
    Vec offset;
    if (side % 2) {
      int x = rand(-game.screen.x, game.screen.x);
      offset = (Vec){x, game.screen.y / (side == 0 ? -1.0 : 1.0)};
    } else {
      int y = rand(-game.screen.y, game.screen.y);
      offset = (Vec){game.screen.x / (side == 1 ? -1.0 : 1.0), y};
    }

    spritesNew(&game.enemies, vecAdd(game.player.position, offset), (Vec){0});

    if (game.spawn.delay > SPAWN_DELAY / 5) {
      game.spawn.delay -= 5;
    }
    timerReset(&game.spawn);
  }

  for (int i = 0; i < game.bullets.count; ++i) {
    Sprite *bullet = &game.bullets.data[i];
    if (bullet->life) {
      spriteMove(bullet);
      if (spriteAway(bullet, game.player.position, DESPAWN_DISTANCE)) {
        spritesFree(&game.bullets, i);
      }
    }
  }

  for (int i = 0; i < game.enemies.count; ++i) {
    Sprite *enemy = &game.enemies.data[i];
    if (enemy->life) {
      enemy->velocity = vecSub(game.player.position, enemy->position);
      enemy->velocity = vecLimit(enemy->velocity, ENEMY_SPEED);

      spriteMove(enemy);
      if (spriteAway(enemy, game.player.position, DESPAWN_DISTANCE)) {
        spritesFree(&game.enemies, i);
      }

      for (int j = 0; j < game.bullets.count; ++j) {
        Sprite *bullet = &game.bullets.data[j];
        if (spriteTouch(enemy, bullet, ENEMY_SIZE + BULLET_SIZE)) {
          game.score++;
          spritesFree(&game.bullets, j);
          spritesFree(&game.enemies, i);
          break;
        }
      }
    }

    if (enemy->life) {
      spriteTouch(enemy, &game.player, ENEMY_SIZE + PLAYER_SIZE);
    }
  }
}
