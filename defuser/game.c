#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PLAYER_SPEED 150.0f
#define TARGET_FPS 120
#define POINT_SIZE 16
#define TILE_SIZE (Vector2){ POINT_SIZE, POINT_SIZE }
#define WORLD_SIZE 20
#define TIME_LIMIT_PER_BOMB .7f

typedef enum {
  BOMB,
  TRAP,
  EMPTY,
  WALL,
} TileType;

typedef struct {
  Rectangle bounds;
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
        tile->type = WALL;
      } else {
        tile->type = EMPTY;

        int rand = GetRandomValue(0, 100);
        // 2% chance to be a trap
        if ( rand < 2 ) {
          tile->type = TRAP;
        // 5% chance to be a bomb
        } else if (rand < 5) {
          tile->type = BOMB;
          tile->defused = false;
          totalBombs++;
        // 5% chance to be a wall
        } else if (rand > 5 && rand < 10) {
          tile->type = WALL;
        } 
      }
    } 
  }

  timeleftSeconds = TIME_LIMIT_PER_BOMB * totalBombs;
}

#define PLAYER_SCALE 0.8f

float worldCenterX = (WORLD_SIZE * TILE_SIZE.x) / 2.0f - (POINT_SIZE * PLAYER_SCALE / 2.0f);
float worldCenterY = (WORLD_SIZE * TILE_SIZE.y) / 2.0f - (POINT_SIZE * PLAYER_SCALE / 2.0f);

Rectangle GetDefaultPlayerBounds() {
  return (Rectangle){
      worldCenterX,
      worldCenterY,
      POINT_SIZE * PLAYER_SCALE,
      POINT_SIZE * PLAYER_SCALE
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
   
    Texture2D trap = LoadTexture("./assets/trap.png");
    Texture2D empty_1 = LoadTexture("./assets/empty_1.png");
    Texture2D empty_2 = LoadTexture("./assets/empty_2.png");
    Texture2D empty_3 = LoadTexture("./assets/empty_3.png");

    Texture2D bomb = LoadTexture("./assets/bomb.png");
    Texture2D bomb_defused = LoadTexture("./assets/bomb_defused.png");

    Texture2D skull = LoadTexture("./assets/skull_1.png");

    Texture2D wall_top, wall_right, wall_left, wall_bottom;
    wall_top = wall_right = wall_left = wall_bottom = LoadTexture("./assets/wall_vertical.png");
    Rectangle playerBounds = GetDefaultPlayerBounds();

    Camera2D camera = { 0 };
    camera.zoom = 2.0f;
    int dir = 1;

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
          dir = -1;
        }
        if (IsKeyDown(KEY_D)) {
          playerBounds.x += GetFrameTime() * PLAYER_SPEED;
          dir = 1;
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
                Tile tile = world[i];
                int t = i % 3;
                Texture2D texture = empty_1;
                if (t == 1) texture = empty_2;
                if (t == 2) texture = empty_3;
                DrawTexture(texture, tile.bounds.x, tile.bounds.y, WHITE);
                if (tile.type == WALL) {
                  if (tile.bounds.x < SCREEN_WIDTH/2.0f) {
                    DrawTexture(wall_left, tile.bounds.x, tile.bounds.y, WHITE);
                  } else {
                    DrawTexture(wall_right, tile.bounds.x, tile.bounds.y, WHITE);
                  }
                  if (tile.bounds.y < SCREEN_HEIGHT/2.0f) {
                    DrawTexture(wall_top, tile.bounds.x, tile.bounds.y, WHITE);
                  }
                } else if (tile.type == TRAP) {
                  DrawTexture(trap, tile.bounds.x, tile.bounds.y, WHITE);
                } else if (tile.type == BOMB) {
                  Texture2D texture = bomb;
                  if (tile.defused == true) {
                    texture = bomb_defused;
                  }
                  DrawTexture(texture, tile.bounds.x, tile.bounds.y, WHITE);
                }
              }
              Rectangle source = { 0.0f, 0.0f, (float)skull.width * dir, (float)skull.height };

              DrawTextureRec(skull, source, (Vector2){ playerBounds.x, playerBounds.y }, WHITE);
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

