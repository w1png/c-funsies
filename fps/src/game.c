#include "const.h"
#include "texture.h"
#include "objects.h"
#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "player.h"
#include "world.h"

typedef enum {
  PLAYING,
} GameState;

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SomeGame");
    InitAudioDevice();
    SetRandomSeed(clock());
    LoadTextures();
    RegisterObjects();

    bool debugEnabled = false;

    Tile world[MAX_WORLD_SIZE * MAX_WORLD_SIZE];
    GameState gameState = PLAYING;
    Player player = {0};
    InitPlayer(&player);
    GenerateWorld(world);

    Camera2D camera = { 0 };
    camera.zoom = 0.5f;
    float deltaY = 0;
    float deltaX = 0;

    Tile *hoveredTile = NULL;

    SetTargetFPS(TARGET_FPS);
    while (!WindowShouldClose()) {
      if (gameState == PLAYING) {
        camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));

        if (camera.zoom > MIN_ZOOM) camera.zoom = MIN_ZOOM;
        else if (camera.zoom < MAX_ZOOM) camera.zoom = MAX_ZOOM;

        if (IsKeyPressed(KEY_BACKSLASH)) {
          debugEnabled = !debugEnabled;
        }

        if (IsKeyPressed(KEY_ONE)) {
          player.selectedInventoryObjectIndex = 0;
        }
        if (IsKeyPressed(KEY_TWO)) {
          player.selectedInventoryObjectIndex = 1;
        }
        if (IsKeyPressed(KEY_THREE)) {
          player.selectedInventoryObjectIndex = 2;
        }
        if (IsKeyPressed(KEY_FOUR)) {
          player.selectedInventoryObjectIndex = 3;
        }
        if (IsKeyPressed(KEY_FIVE)) {
          player.selectedInventoryObjectIndex = 4;
        }

        hoveredTile = NULL;
        bool hasCollidedWall = false;
        Rectangle playerPositionBefore = player.bounds;

        deltaY = 0;
        deltaX = 0;
        if (IsKeyDown(KEY_A)) {
          deltaX -= PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_D)) {
          deltaX += PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_S)) {
          deltaY += PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_W)) {
          deltaY -= PLAYER_SPEED;
        }
        float runingMultiplier = (IsKeyDown(KEY_LEFT_SHIFT)) ? RUN_MULTIPLIER : 1.0f;

        player.bounds.x += deltaX * GetFrameTime() * runingMultiplier;
        player.bounds.y += deltaY * GetFrameTime() * runingMultiplier;

        for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
            Tile *tile = &world[i];

            bool isBreaking = false;
            bool isColliding = CheckCollisionRecs(player.bounds, tile->bounds);
            if (CheckCollisionPointRec(GetScreenToWorld2D(GetMousePosition(), camera), tile->bounds)) {
              float distance = Vector2Distance(
                (Vector2){
                  player.bounds.x + player.bounds.width / 2.0f,
                  player.bounds.y + player.bounds.height / 2.0f
                }, 
                (Vector2){
                  tile->bounds.x + tile->bounds.width / 2.0f,
                  tile->bounds.y + tile->bounds.height / 2.0f
                }
              );

              hoveredTile = tile;
              if (distance <= (BREAK_DISTANCE * POINT_SIZE)) {
                  if (HAS_TAG(tile->object, TAG_BREAKABLE) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    isBreaking = true;
                  }

                  if (HAS_TAG(tile->object, TAG_EMPTY) && IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) { 
                    InventoryObject *inventoryObject = player.inventory[player.selectedInventoryObjectIndex];
                    if (inventoryObject != NULL && HAS_TAG(inventoryObject->object, TAG_PLACEABLE)) {
                      tile->object = inventoryObject->object;
                      inventoryObject->amount--;
                      HandleUpdateInventory(&player);
                    }
                  }
              }
            }

            if (isBreaking) {
              tile->breakTimeSecondsPassed += GetFrameTime();
              if (tile->breakTimeSecondsPassed >= tile->object->breakTimeSeconds) {
                if (tile->object->onBreak != NULL) {
                  tile->object->onBreak(tile->object, tile, &player);
                }
                continue;
              }
            } else {
              tile->breakTimeSecondsPassed = 0;
            }

            if (isColliding && !hasCollidedWall) {
              hasCollidedWall = HAS_TAG(tile->object, TAG_BLOCKING);
              continue;
            }
        }

        if (hasCollidedWall) {
          player.bounds = playerPositionBefore;
        }

        camera.target = (Vector2){ player.bounds.x + 20.0f, player.bounds.y + 20.0f };
        camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
      }

      BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (gameState) {
          case PLAYING: {
            BeginMode2D(camera);
              for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
                Tile tile = world[i];
                Texture2D tex = HAS_TAG(tile.object, TAG_BACKGROUND) ? *tile.object->texture : textures.grass;

                DrawTexturePro(
                 tex, 
                 (Rectangle){0, 0, (float)tex.width, (float)tex.height}, 
                 (Rectangle){tile.bounds.x, tile.bounds.y, (float)POINT_SIZE, (float)POINT_SIZE}, 
                 (Vector2){0,0}, 
                 0.0f, WHITE
                );
              }

              DrawPlayer(&player, (Vector2){deltaX, deltaY});

              for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
                Tile tile = world[i];
                if (HAS_TAG(tile.object, TAG_BACKGROUND)) continue;
                if (!tile.object->texture) continue;

                Texture2D tex = *tile.object->texture;

                DrawTexturePro(
                 tex, 
                 (Rectangle){0, 0, (float)tex.width, (float)tex.height}, 
                 (Rectangle){tile.bounds.x, tile.bounds.y, (float)POINT_SIZE, (float)POINT_SIZE}, 
                 (Vector2){0,0}, 
                 0.0f, WHITE
                );
              }

              if (hoveredTile != NULL) {
                DrawRectangle(hoveredTile->bounds.x, hoveredTile->bounds.y, hoveredTile->bounds.width, hoveredTile->bounds.height, (Color){0,255,0,100});
              }
            EndMode2D();
            break;
          }
        }

        if (debugEnabled) {
          const char *fpsText = 0;
          if (TARGET_FPS <= 0) fpsText = TextFormat("FPS: unlimited (%i)", GetFPS());
          else fpsText = TextFormat("FPS: %i (target: %i)", GetFPS(), TARGET_FPS);
          DrawText(fpsText, 10, SCREEN_HEIGHT-20, 10, DARKGRAY);
          DrawText(TextFormat("Frame time: %02.02f ms", GetFrameTime()), 10, SCREEN_HEIGHT-30, 10, DARKGRAY);

          if (hoveredTile != NULL) {
            DrawText(TextFormat("Tile type: %s", hoveredTile->object->name), 10, 64, 10, DARKGRAY);
          }
        }

        DrawInventory(&player);
        
        if (debugEnabled) {
          for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
            Tile tile = world[i];
            char text[32];
            snprintf(text, sizeof(text), "%.1f/%.1f", tile.breakTimeSecondsPassed, tile.object->breakTimeSeconds);

            DrawTextEx(
              GetFontDefault(),
              text, 
              GetWorldToScreen2D(
                (Vector2){
                  tile.bounds.x + tile.bounds.width / 2.0f - 6,
                  tile.bounds.y + tile.bounds.height / 2.0f - 6
                }, 
                camera
              ),
              6,
              1,
              WHITE
            );
          }
      }

      EndDrawing();
    }

    CloseWindow();

    return 0;
}
