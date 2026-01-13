#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PLAYER_SPEED 300.0f
#define TARGET_FPS 120
#define POINT_SIZE 32
#define TILE_SIZE (Vector2){ POINT_SIZE, POINT_SIZE }
#define WORLD_SIZE 20
#define TIME_LIMIT_PER_BOMB .5f

#define COLOR_BOUNDS (Color){ 0, 0, 0, 255 }
#define COLOR_TRAP (Color){ 255, 0, 0, 255 }
#define COLOR_DEFUSED (Color){ 255, 255, 255, 255 }

typedef struct {
  Rectangle bounds;
  Color color;
} Tile;

Tile world[WORLD_SIZE * WORLD_SIZE];
int totalTraps = 0;
int trapsDefused = 0;
float timeleftSeconds = 0.0f;

void initGame() {
  SetRandomSeed(clock());
  totalTraps = 0;
  trapsDefused = 0;

  for (int i = 0; i < WORLD_SIZE; i++) {
    for (int j = 0; j < WORLD_SIZE; j++) {
      Tile *tile = &world[i*(int)WORLD_SIZE+j];
      tile->bounds = (Rectangle){ i*TILE_SIZE.x, j*TILE_SIZE.y, TILE_SIZE.x, TILE_SIZE.y };

      // if end of world
      if (i == 0 || i == WORLD_SIZE-1 || j == 0 || j == WORLD_SIZE-1) {
        tile->color = (Color){ 0, 0, 0, 255 };
      } else {
        // 5% chance to be red (a trap)
        if (GetRandomValue(0, 100) < 5) {
          tile->color = (Color){ 255, 0, 0, 255 };
          totalTraps++;
        } else {
          tile->color = (Color){ 255, 255, 255, 255 };
        }
      }
    } 
  }

  timeleftSeconds = TIME_LIMIT_PER_BOMB * totalTraps;
}

Rectangle GetDefaultPlayerBounds() {
  return (Rectangle){ SCREEN_WIDTH/2.0f - POINT_SIZE/2.0f, SCREEN_HEIGHT/2.0f - POINT_SIZE/2.0f, POINT_SIZE, POINT_SIZE };
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

    Rectangle playerBounds = GetDefaultPlayerBounds();

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    SetTargetFPS(TARGET_FPS);
    while (!WindowShouldClose()) {
      switch (gameState) {
        case LOSE:
        case DONE:
        case TITLE: {
          if (IsKeyPressed(KEY_SPACE)) {
            gameState = PLAYING;
            playerBounds = GetDefaultPlayerBounds();
            initGame();
          }
          break;
        }
        case PLAYING: {
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
                  bool isWall = tile->color.r == 0 && tile->color.g == 0 && tile->color.b == 0;

                  if (CheckCollisionRecs(playerBounds, world[i].bounds)) {
                    if (isWall) {
                      hasCollidedWall = true;
                      continue;
                    };

                    if (tile->color.r == 255 && tile->color.g == 0 && tile->color.b == 0) {
                      tile->color = (Color){ 0, 255, 0, 120 };
                      trapsDefused++;
                      if (trapsDefused >= totalTraps) {
                        gameState = DONE;
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
              }
          break;
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

            DrawText(TextFormat("Traps found: %d/%d", trapsDefused, totalTraps), 10, 10, 10, DARKGRAY);
            DrawText(TextFormat("Time left: %0.2fs", timeleftSeconds), 10, 30, 10, DARKGRAY);
            break;
          }
          case DONE: {
            const char* winText = TextFormat("You win! (Found %d traps!)", trapsDefused);
            DrawText(winText, SCREEN_WIDTH/2.0f - MeasureText(winText, 20)/2.0f, SCREEN_HEIGHT/2.0f, 20, DARKGRAY);
            DrawText("Press SPACE to restart", SCREEN_WIDTH/2.0f - MeasureText("Press SPACE to restart", 20)/2.0f, SCREEN_HEIGHT/2.0f + 40, 20, DARKGRAY);
            
            break;
          }
          case LOSE: {
            const char* loseText = TextFormat("You lose :c (Found %d/%d traps!)", trapsDefused, totalTraps);
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

