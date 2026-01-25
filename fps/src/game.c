#include "camera.h"
#include "const.h"
#include "crafting.h"
#include "lib/partikel.h"
#include "texture.h"
#include "objects.h"
#include "particle.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "player.h"
#include "ui.h"
#include "world.h"
#include "gamestate.h"

GameState gameState = {
  .tags = TAG_PLAYING,
};

typedef struct {
  Player* player;
  Tile* world;
} GameUpdateData;

void HandleGameUpdate(GameUpdateData* data) {
  SetScreenOpen(uiScreens.inventoryHUD, true);

  HandleCameraZoom();
  HandleInventoryButtons(data->player);
  ParticleSystem_Update(particleSystem, GetFrameTime());
  Rectangle initialPlayerPosition = data->player->bounds;

  HandlePlayerMovement(data->player);

  for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
    Tile *tile = &data->world[i];

    bool canBreakTile = CanBreakTile(data->player, tile);
    bool isColliding = CheckPlayerCollision(data->player, tile);

    if (isColliding && HAS_TAG(tile->object, TAG_BLOCKING)) {
      data->player->bounds = initialPlayerPosition;
    }

    tile->isHovered = IsHoveringTile(data->player, tile);

    if (tile->isHovered) {
      bool wasPlaced = false;
      if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && CanPlaceTile(data->player, tile)) {
        wasPlaced = HandlePlaceTile(data->player, tile);
      }

      if (!wasPlaced && IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CanInteractTile(data->player, tile)) {
        tile->object->onClick(tile->object, tile, data->player);
      }
    }

    if (canBreakTile && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      HandleBreakTile(data->player, tile);
    } else {
      tile->breakTimeSecondsPassed = 0;
    }
  }

  HandleCameraTarget(data->player);
}

typedef struct {
  Tile* world;
  Player* player;
} DrawData;

void Draw(DrawData data) {
  Tile* world = data.world;
  Player* player = data.player;

  BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode2D(camera);
      DrawWorldBackground(world);
      DrawPlayer(player);
      DrawWorldForeground(world);

      ParticleSystem_Draw(particleSystem);
    EndMode2D();

    HandleAllUIUpdates();

  EndDrawing();
}


void InitGame() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SomeGame");
  InitCamera();
  InitAudioDevice();
  SetRandomSeed(clock());
  SetExitKey(0);

  LoadTextures();
  RegisterAllObjects();
  RegisterAllCraftingRecipes();
  RegisterAllUIScreens();
  RegisterEmitterConfigs();
}

int main(void) {
  InitGame();

  Tile world[MAX_WORLD_SIZE * MAX_WORLD_SIZE];

  Player player = {0};
  InitPlayer(&player);
  GenerateWorld(world, player.bounds);

  bool isExiting = false;
  PauseMenuData pauseMenuData = { &isExiting };
  uiScreens.pauseMenu->data = &pauseMenuData;

  PlayerInventoryData playerInventoryData = {.player = &player};
  uiScreens.inventoryHUD->data = &playerInventoryData;

  DebugMenuData debugMenuData = { .world = world, .player = &player};
  uiScreens.debugMenu->data = &debugMenuData;

  GameUpdateData gameUpdateData = {
    .player = &player,
    .world = world,
  };
  uiScreens.debugHUD->data = &debugMenuData;

  InventoryMenuData inventoryMenuData = { .player = &player };
  uiScreens.inventoryMenu->data = &inventoryMenuData;

  DrawData drawData = {
    .world = world,
    .player = &player,
  };

  SetTargetFPS(TARGET_FPS);
  while (!WindowShouldClose() && !isExiting) {
    if (IsKeyPressed(KEY_E)) {
      SetScreenOpen(uiScreens.inventoryMenu, true);
    }
    if (IsKeyPressed(KEY_BACKSLASH)) {
      SetScreenOpen(uiScreens.debugMenu, true);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      UIScreen* lock = GetUILock();
      if (lock == NULL) {
        SetScreenOpen(uiScreens.pauseMenu, true);
      } else {
        SetScreenOpen(lock, false);
      }
    }

    if (HAS_TAG(&gameState, TAG_PLAYING) && GetUILock() == NULL) {
      HandleGameUpdate(&gameUpdateData);
    }

    Draw(drawData);
  }

  CloseWindow();

  return 0;
}
