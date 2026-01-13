#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PLAYER_SPEED 250.0f
#define TARGET_FPS 120
#define POINT_SIZE 32
#define TILE_SIZE (Vector2){ POINT_SIZE, POINT_SIZE }
#define WORLD_SIZE 20
#define TIME_LIMIT_PER_BOMB .7f

#define COLOR_BOUNDS (Color){ 0, 0, 0, 255 }
#define COLOR_TRAP (Color){ 255, 0, 0, 255 }
#define COLOR_DEFUSED (Color){ 255, 255, 255, 255 }
#define COLOR_BOMB (Color){ 0, 0, 255, 255 }
#define COLOR_BOMB_DEFUSED (Color){ 0, 0, 255, 120 }
#define COLOR_BOMB_TRAP (Color){ 0, 0, 255, 255 }

typedef enum {
  BOMB,
  TRAP,
  EMPTY,
  WALL,
} TileType;

typedef struct {
  Rectangle bounds;
  Color color;
  TileType type;
  bool defused;
} Tile;

Tile world[WORLD_SIZE * WORLD_SIZE];
int totalBombs = 0;
int bombsDefused = 0;
float timeleftSeconds = 0.0f;

void initGame() {
  SetRandomSeed(clock());
  totalBombs = 0;
  bombsDefused = 0;

  for (int i = 0; i < WORLD_SIZE; i++) {
    for (int j = 0; j < WORLD_SIZE; j++) {
      Tile *tile = &world[i*(int)WORLD_SIZE+j];
      tile->bounds = (Rectangle){ i*TILE_SIZE.x, j*TILE_SIZE.y, TILE_SIZE.x, TILE_SIZE.y };

      // if end of world
      if (i == 0 || i == WORLD_SIZE-1 || j == 0 || j == WORLD_SIZE-1) {
        tile->color = (Color){ 0, 0, 0, 255 };
        tile->type = WALL;
      } else {
        tile->type = EMPTY;
        // 5% chance to be a bomb
        if (GetRandomValue(0, 100) < 5) {
          tile->color = COLOR_BOMB;
          tile->type = BOMB;
          totalBombs++;
        // 2% chance to be a trap
        } else if (GetRandomValue(0, 100) < 2) {
          tile->color = COLOR_TRAP;
          tile->type = TRAP;
        } else {
          tile->color = (Color){ 255, 255, 255, 255 };
        }
      }
    } 
  }

  timeleftSeconds = TIME_LIMIT_PER_BOMB * totalBombs;
}

#define PLAUER_SCALE 0.8f

Rectangle GetDefaultPlayerBounds() {
  return (Rectangle){ 
    (SCREEN_WIDTH/2.0f - POINT_SIZE/2.0f) * PLAUER_SCALE,
    (SCREEN_HEIGHT/2.0f - POINT_SIZE/2.0f) * PLAUER_SCALE,
    POINT_SIZE * PLAUER_SCALE,
    POINT_SIZE * PLAUER_SCALE
  };
}

typedef enum {
  TITLE,
  PLAYING,
  DONE,
  LOSE,
} GameState;

int main(void)
{
    GameState gameState = TITLE;

    initGame();
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trap defuser");
    InitAudioDevice();

    Sound defuse = LoadSound("./assets/defuse.wav");
    Sound win = LoadSound("./assets/win.wav");
    Sound lose = LoadSound("./assets/lose.wav");

    Rectangle playerBounds = GetDefaultPlayerBounds();

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    SetTargetFPS(TARGET_FPS);
    while (!WindowShouldClose()) {
      if (IsKeyPressed(KEY_SPACE)) {
        gameState = PLAYING;
        playerBounds = GetDefaultPlayerBounds();
        initGame();
      }

      if (gameState == PLAYING) {
        bool hasCollidedWall = false;
        Rectangle playerPositionBefore = playerBounds;
        if (IsKeyDown(KEY_A)) {
          playerBounds.x -= GetFrameTime() * PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_D)) {
          playerBounds.x += GetFrameTime() * PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_S)) {
          playerBounds.y += GetFrameTime() * PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_W)) {
          playerBounds.y -= GetFrameTime() * PLAYER_SPEED;
        }

        for (int i = 0; i < WORLD_SIZE*WORLD_SIZE; i++) {
            Tile *tile = &world[i];

            if (CheckCollisionRecs(playerBounds, world[i].bounds)) {
              if (tile->type == WALL) {
                hasCollidedWall = true;
                continue;
              };

              if (tile->type == TRAP) {
                PlaySound(lose);
                gameState = LOSE;
                continue;
              }
              if (tile->type == BOMB && !tile->defused) {
                tile->color = COLOR_BOMB_DEFUSED;
                tile->defused = true;
                bombsDefused++;
                PlaySound(defuse);
                if (bombsDefused >= totalBombs) {
                  gameState = DONE;
                  PlaySound(win);
                }
              }
            }
        }

        if (hasCollidedWall) {
          playerBounds = playerPositionBefore;
        }

        camera.target = (Vector2){ playerBounds.x + 20.0f, playerBounds.y + 20.0f };
        camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
        timeleftSeconds -= GetFrameTime();
        if (timeleftSeconds <= 0) {
          gameState = LOSE;
          PlaySound(lose);
        }
      }

      BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (gameState) {
          case TITLE: {
            DrawText("Press SPACE to start", SCREEN_WIDTH/2.0f - MeasureText("Press SPACE to start", 20)/2.0f, SCREEN_HEIGHT/2.0f, 20, DARKGRAY);
            break;
          }
          case PLAYING: {
            BeginMode2D(camera);
              for (int i = 0; i < WORLD_SIZE*WORLD_SIZE; i++) {
                DrawRectangleRec(world[i].bounds, world[i].color);
              }
              DrawRectangleRec(playerBounds, GREEN);
            EndMode2D();

            DrawText(TextFormat("Traps found: %d/%d", bombsDefused, totalBombs), 10, 10, 10, DARKGRAY);
            DrawText(TextFormat("Time left: %0.2fs", timeleftSeconds), 10, 30, 10, DARKGRAY);
            break;
          }
          case DONE: {
            const char* winText = TextFormat("You win! (Found %d traps!)", bombsDefused);
            DrawText(winText, SCREEN_WIDTH/2.0f - MeasureText(winText, 20)/2.0f, SCREEN_HEIGHT/2.0f, 20, DARKGRAY);
            DrawText("Press SPACE to restart", SCREEN_WIDTH/2.0f - MeasureText("Press SPACE to restart", 20)/2.0f, SCREEN_HEIGHT/2.0f + 40, 20, DARKGRAY);
            
            break;
          }
          case LOSE: {
            const char* loseText = TextFormat("You lose :c (Found %d/%d traps!)", bombsDefused, totalBombs);
            DrawText(loseText, SCREEN_WIDTH/2.0f - MeasureText(loseText, 20)/2.0f, SCREEN_HEIGHT/2.0f, 20, DARKGRAY);
            DrawText("Press SPACE to restart", SCREEN_WIDTH/2.0f - MeasureText("Press SPACE to restart", 20)/2.0f, SCREEN_HEIGHT/2.0f + 40, 20, DARKGRAY);
            break;
          }
        }

        const char *fpsText = 0;
        if (TARGET_FPS <= 0) fpsText = TextFormat("FPS: unlimited (%i)", GetFPS());
        else fpsText = TextFormat("FPS: %i (target: %i)", GetFPS(), TARGET_FPS);
        DrawText(fpsText, 10, SCREEN_HEIGHT-20, 10, DARKGRAY);
        DrawText(TextFormat("Frame time: %02.02f ms", GetFrameTime()), 10, SCREEN_HEIGHT-30, 10, DARKGRAY);

      EndDrawing();
    }

    CloseWindow();

    return 0;
}

