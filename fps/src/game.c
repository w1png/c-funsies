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
#include "ui.h"
#include "world.h"
#include "gamestate.h"

Tile* hoveredTile = NULL;
GameState gameState = {
  .tags = TAG_PLAYING,
};
Camera2D camera = { 0 };

typedef struct {
  Player* player;
  Tile* world;
} GameUpdateData;

typedef struct {
  float deltaY;
  float deltaX;
} GameUpdateResult;

void HandleGameUpdate(GameUpdateData* data, GameUpdateResult* result) {
  SetScreenOpen(playerInventoryMenu, true);

  camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));

  if (camera.zoom > MIN_ZOOM) camera.zoom = MIN_ZOOM;
  else if (camera.zoom < MAX_ZOOM) camera.zoom = MAX_ZOOM;

  if (IsKeyPressed(KEY_ONE)) {
    data->player->selectedInventoryObjectIndex = 0;
  }
  if (IsKeyPressed(KEY_TWO)) {
    data->player->selectedInventoryObjectIndex = 1;
  }
  if (IsKeyPressed(KEY_THREE)) {
    data->player->selectedInventoryObjectIndex = 2;
  }
  if (IsKeyPressed(KEY_FOUR)) {
    data->player->selectedInventoryObjectIndex = 3;
  }
  if (IsKeyPressed(KEY_FIVE)) {
    data->player->selectedInventoryObjectIndex = 4;
  }

  hoveredTile = NULL;
  bool hasCollidedWall = false;
  Rectangle playerPositionBefore = data->player->bounds;

  result->deltaY = 0;
  result->deltaX = 0;
  if (IsKeyDown(KEY_A)) {
    result->deltaX -= PLAYER_SPEED;
  }
  if (IsKeyDown(KEY_D)) {
    result->deltaX += PLAYER_SPEED;
  }
  if (IsKeyDown(KEY_S)) {
    result->deltaY += PLAYER_SPEED;
  }
  if (IsKeyDown(KEY_W)) {
    result->deltaY -= PLAYER_SPEED;
  }
  float runingMultiplier = (IsKeyDown(KEY_LEFT_SHIFT)) ? RUN_MULTIPLIER : 1.0f;

  data->player->bounds.x += result->deltaX * GetFrameTime() * runingMultiplier;
  data->player->bounds.y += result->deltaY * GetFrameTime() * runingMultiplier;

  for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
      Tile *tile = &data->world[i];

      bool isBreaking = false;
      bool isColliding = CheckCollisionRecs(data->player->bounds, tile->bounds);
      if (CheckCollisionPointRec(GetScreenToWorld2D(GetMousePosition(), camera), tile->bounds)) {
        float distance = Vector2Distance(
          (Vector2){
            data->player->bounds.x + data->player->bounds.width / 2.0f,
            data->player->bounds.y + data->player->bounds.height / 2.0f
          }, 
          (Vector2){
            tile->bounds.x + tile->bounds.width / 2.0f,
            tile->bounds.y + tile->bounds.height / 2.0f
          }
        );

        hoveredTile = tile;
        if (distance <= (INTERACT_DISTANCE * POINT_SIZE)) {
            if (HAS_TAG(tile->object, TAG_BREAKABLE) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
              isBreaking = true;
            }

            if (HAS_TAG(tile->object, TAG_EMPTY) && IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) { 
              InventoryObject *inventoryObject = data->player->inventory[data->player->selectedInventoryObjectIndex];
              if (
                inventoryObject != NULL && HAS_TAG(inventoryObject->object, TAG_PLACEABLE) && 
                !CheckCollisionRecs(data->player->bounds, tile->bounds)
              ) {
                tile->object = inventoryObject->object;
                inventoryObject->amount--;
                HandleUpdateInventory(data->player);
              }
            }
        }
      }

      if (isBreaking) {
        tile->breakTimeSecondsPassed += GetFrameTime();
        if (tile->breakTimeSecondsPassed >= tile->object->breakTimeSeconds) {
          if (tile->object->onBreak != NULL) {
            tile->object->onBreak(tile->object, tile, data->player);
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
    data->player->bounds = playerPositionBefore;
  }
  
  camera.target = (Vector2){ data->player->bounds.x + 20.0f, data->player->bounds.y + 20.0f };
  camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SomeGame");
    InitAudioDevice();
    SetRandomSeed(clock());
    SetExitKey(0);

    LoadTextures();
    RegisterObjects();
    RegisterAllUIScreens();

    bool debugEnabled = false;

    Tile world[MAX_WORLD_SIZE * MAX_WORLD_SIZE];

    Player player = {0};
    InitPlayer(&player);
    GenerateWorld(world, player.bounds);

    camera.zoom = 0.5f;

    bool isExiting = false;
    PauseMenuData pauseMenuData = { &isExiting };
    pauseMenu->data = &pauseMenuData;

    PlayerInventoryData playerInventoryData = {.player = &player};
    playerInventoryMenu->data = &playerInventoryData;

    DebugMenuData debugMenuData = { .world = world, .player = &player, .hoveredTile = &hoveredTile, .camera = &camera};
    debugMenu->data = &debugMenuData;

    GameUpdateData gameUpdateData = {
      .player = &player,
      .world = world,
    };
    debugInfo->data = &debugMenuData;

    GameUpdateResult gameUpdateResult = {};

    SetTargetFPS(TARGET_FPS);
    while (!WindowShouldClose() && !isExiting) {
      if (IsKeyPressed(KEY_BACKSLASH)) {
        SetScreenOpen(debugMenu, !debugMenu->isOpen);
      }
      if (IsKeyPressed(KEY_ESCAPE)) {
        SetScreenOpen(pauseMenu, !pauseMenu->isOpen);
      }

      if (HAS_TAG(&gameState, TAG_PLAYING) && GetUILock() == NULL) {
        HandleGameUpdate(&gameUpdateData, &gameUpdateResult);
      }

      BeginDrawing();
        ClearBackground(RAYWHITE);

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

          DrawPlayer(&player, (Vector2){gameUpdateResult.deltaX, gameUpdateResult.deltaY});

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
        HandleAllUIUpdates();


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
