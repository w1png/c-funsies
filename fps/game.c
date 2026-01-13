#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PLAYER_SPEED 50.0f
#define RUN_MULTIPLIER 1.5f
#define TARGET_FPS 120
#define POINT_SIZE 16
#define TILE_SIZE (Vector2){ POINT_SIZE, POINT_SIZE }
#define MAX_WORLD_SIZE 100
#define TIME_LIMIT_PER_BOMB .7f
#define PLAYER_SCALE 0.8f
#define MAX_INVENTORY_ITEMS 5
#define BREAK_DISTANCE 3.0f

Texture2D missingTexture;
Texture2D playerTexture;
Texture2D wallTexture;
Texture2D treeTexture;
Texture2D waterTexture;
Texture2D grassTexture;
Texture2D stoneTexture;
Texture2D woodTexture;
void LoadTextures() {
  missingTexture = LoadTexture("./assets/missing_texture.png");
  if (!IsTextureValid(missingTexture)) {
    TraceLog(LOG_ERROR, "Failed to load texture assets/missing_texture.png");
  }

  playerTexture = LoadTexture("./assets/player_texture.png");
  if (!IsTextureValid(playerTexture)) {
    TraceLog(LOG_ERROR, "Failed to load texture assets/player.png");
  }

  wallTexture = LoadTexture("./assets/wall_texture.png");
  if (!IsTextureValid(wallTexture)) {
    TraceLog(LOG_ERROR, "Failed to load texture assets/wall_texture.png");
  }

  treeTexture = LoadTexture("./assets/tree_texture.png");
  if (!IsTextureValid(treeTexture)) {
    TraceLog(LOG_ERROR, "Failed to load texture assets/tree_texture.png");
  }

  waterTexture = LoadTexture("./assets/water_texture.png");
  if (!IsTextureValid(waterTexture)) {
    TraceLog(LOG_ERROR, "Failed to load texture assets/water_texture.png");
  }

  grassTexture = LoadTexture("./assets/grass_texture.png");
  if (!IsTextureValid(grassTexture)) {
    TraceLog(LOG_ERROR, "Failed to load texture assets/grass_texture.png");
  }

  stoneTexture = LoadTexture("./assets/stone_texture.png");
  if (!IsTextureValid(stoneTexture)) {
    TraceLog(LOG_ERROR, "Failed to load texture assets/stone_texture.png");
  }

  woodTexture = LoadTexture("./assets/wood_texture.png");
  if (!IsTextureValid(woodTexture)) {
    TraceLog(LOG_ERROR, "Failed to load texture assets/wood_texture.png");
  }
}

typedef enum {
  TT_EMPTY,
  TT_WALL,
  TT_TREE,
  TT_STONE,
  TT_WATER,
  TT_WOOD,
} TileTypeEnum;

typedef struct {
  Texture2D *texture;
  const char* name;
  bool isPlaceable;
} Item;

Item wood = {
  .name = "wood",
  .texture = &woodTexture,
  .isPlaceable = true,
};

typedef struct {
  Item item;
  int amount;
} InventoryItem;

typedef struct {
  Rectangle bounds;
  InventoryItem* inventory[MAX_INVENTORY_ITEMS];
  int selectedInventoryItemIndex;
} Player;

void AddToInventory(Item *item, int amount, Player *player) {
  bool hasItem = false;
  int firstFreeSlot = -1;
  for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) {
    InventoryItem *inventoryItem = player->inventory[i];
    if (inventoryItem == NULL) {
      firstFreeSlot = i;
      break;
    }

    if (inventoryItem->item.name == item->name) {
      inventoryItem->amount += amount;
      hasItem = true;
    }
  }

  if (!hasItem) {
    if (firstFreeSlot == -1) {
      TraceLog(LOG_ERROR, "No free inventory slot");
      return;
    }

    player->inventory[firstFreeSlot] = (InventoryItem*)malloc(sizeof(InventoryItem));
    player->inventory[firstFreeSlot]->item = *item;
    player->inventory[firstFreeSlot]->amount = amount;
  }
}

typedef struct  {
  const char *name;
  TileTypeEnum type;
  bool isBlocking;
  Texture2D *texture;

  void* (*onBreak)(void *data, Player *player);
  bool isBreakable;
  bool isPlaceable;
  float breakTimeSeconds;
} TileType;

typedef struct {
  Rectangle bounds;
  TileType *type;

  float breakTimeSecondsPassed;
} Tile;


TileType EMPTY = {
  .name = "EMPTY",
  .type = TT_EMPTY,
  .isBreakable = false,
  .isBlocking = false,
  .breakTimeSeconds = 0.0f,
  .isPlaceable = true,
  .texture = &grassTexture,
};

TileType WALL = {
  .name = "WALL",
  .type = TT_WALL,
  .isBreakable = false,
  .isBlocking = true,
  .breakTimeSeconds = 0.0f,
  .isPlaceable = false,
  .texture = &wallTexture,
};

void* OnBreakTree(void *data, Player *player) {
  Tile *tile = (Tile*)data;
  AddToInventory(&wood, GetRandomValue(1, 3), player);
  tile->type = &EMPTY;
  return NULL;
}

TileType TREE = {
  .name = "TREE",
  .type = TT_TREE,
  .isBreakable = true,
  .isBlocking = true,
  .breakTimeSeconds = 1.0f,
  .texture = &treeTexture,
  .isPlaceable = false,
  .onBreak = OnBreakTree,
};

void* OnBreakWood(void *data, Player *player) {
  Tile *tile = (Tile*)data;
  AddToInventory(&wood, 1, player);
  tile->type = &EMPTY;
  return NULL;
}

TileType WOOD = {
  .name = "WOOD",
  .type = TT_WOOD,
  .isBreakable = true,
  .isBlocking = true,
  .breakTimeSeconds = 0.3f,
  .isPlaceable = false,
  .texture = &woodTexture,
  .onBreak = OnBreakWood,
};

TileType STONE = {
  .name = "STONE",
  .type = TT_STONE,
  .isBreakable = true,
  .isBlocking = true,
  .breakTimeSeconds = 1.0f,
  .isPlaceable = false,
  .texture = &stoneTexture,
};

TileType WATER = {
  .name = "WATER",
  .type = TT_WATER,
  .isBreakable = false,
  .isBlocking = true,
  .breakTimeSeconds = 0.0f,
  .isPlaceable = false,
  .texture = &waterTexture,
};

Tile world[MAX_WORLD_SIZE * MAX_WORLD_SIZE];

void InitPlayer(Player *player) {
  player->bounds = (Rectangle){
    0.0f,
    0.0f,
    POINT_SIZE * PLAYER_SCALE,
    POINT_SIZE * PLAYER_SCALE
  };
  player->selectedInventoryItemIndex = 0;
}

void InitWorld(Tile world[MAX_WORLD_SIZE * MAX_WORLD_SIZE]) {
  for (int i = 0; i < MAX_WORLD_SIZE; i++) {
    for (int j = 0; j < MAX_WORLD_SIZE; j++) {
      Tile *tile = &world[i*(int)MAX_WORLD_SIZE+j];
      tile->bounds = (Rectangle){ i*TILE_SIZE.x, j*TILE_SIZE.y, TILE_SIZE.x, TILE_SIZE.y };
      tile->type = &EMPTY;

      if (GetRandomValue(0, 100) < 5) {
        tile->type = &TREE;
      } else if (GetRandomValue(0, 100) < 5) {
        tile->type = &STONE;
      }
    }
  }
}

typedef enum {
  PLAYING,
} GameState;

void DrawDebugInfo() {
  
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trap defuser");
    InitAudioDevice();
    SetRandomSeed(clock());
    LoadTextures();

    bool debugEnabled = false;

    GameState gameState = PLAYING;
    Player player = {0};
    InitPlayer(&player);
    InitWorld(world);

    Camera2D camera = { 0 };
    camera.zoom = 2.0f;
    int dir = 1;

    Tile *hoveredTile = NULL;

    SetTargetFPS(TARGET_FPS);
    while (!WindowShouldClose()) {
      if (gameState == PLAYING) {
        if (IsKeyPressed(KEY_BACKSLASH)) {
          debugEnabled = !debugEnabled;
        }

        if (IsKeyPressed(KEY_ONE)) {
          player.selectedInventoryItemIndex = 0;
        }
        if (IsKeyPressed(KEY_TWO)) {
          player.selectedInventoryItemIndex = 1;
        }
        if (IsKeyPressed(KEY_THREE)) {
          player.selectedInventoryItemIndex = 2;
        }
        if (IsKeyPressed(KEY_FOUR)) {
          player.selectedInventoryItemIndex = 3;
        }
        if (IsKeyPressed(KEY_FIVE)) {
          player.selectedInventoryItemIndex = 4;
        }

        hoveredTile = NULL;
        bool hasCollidedWall = false;
        Rectangle playerPositionBefore = player.bounds;

        float deltaY = 0;
        float deltaX = 0;

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
                  if (tile->type->isBreakable && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                      isBreaking = true;
                  }

                  if (tile->type->isPlaceable && IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
                    // TODO: implement item.onPlace(&tile, &player);
                    InventoryItem* item = player.inventory[player.selectedInventoryItemIndex];
                    if (item != NULL) {
                      tile->type = &WOOD;
                      item->amount--;
                      if (item->amount == 0) {
                        player.inventory[player.selectedInventoryItemIndex] = NULL;
                      }
                    }
                  }
              }
            }

            if (isBreaking) {
              tile->breakTimeSecondsPassed += GetFrameTime();
              if (tile->breakTimeSecondsPassed >= tile->type->breakTimeSeconds) {
                if (tile->type->onBreak != NULL) {
                  tile->type->onBreak(tile, &player);
                }
                continue;
              }
            } else {
              tile->breakTimeSecondsPassed = 0;
            }

            if (isColliding && !hasCollidedWall) {
              hasCollidedWall = tile->type->isBlocking;
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
                DrawTexture(grassTexture, tile.bounds.x, tile.bounds.y, WHITE);
              }

              Rectangle source = { 0.0f, 0.0f, (float)playerTexture.width * dir, (float)playerTexture.height };
              DrawTextureRec(playerTexture, source, (Vector2){ player.bounds.x, player.bounds.y }, WHITE);

              for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
                Tile tile = world[i];
                Texture2D *texture = tile.type->texture;
                if (tile.type->type == TT_EMPTY) continue;
                Vector2 tileCenter = {
                    tile.bounds.x + tile.bounds.width,
                    tile.bounds.y + tile.bounds.height
                };

                Vector2 origin = {
                    texture->width - floor(((float)texture->width / POINT_SIZE / 2)) * POINT_SIZE,
                    texture->height
                };

                DrawTexturePro(
                  *texture,
                  (Rectangle){ 0, 0, (float)texture->width, (float)texture->height },
                  (Rectangle){ tileCenter.x, tileCenter.y, (float)texture->width, (float)texture->height },
                  origin,
                  0.0f,
                  WHITE
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
          DrawDebugInfo();
        }
        const char *fpsText = 0;
        if (TARGET_FPS <= 0) fpsText = TextFormat("FPS: unlimited (%i)", GetFPS());
        else fpsText = TextFormat("FPS: %i (target: %i)", GetFPS(), TARGET_FPS);
        DrawText(fpsText, 10, SCREEN_HEIGHT-20, 10, DARKGRAY);
        DrawText(TextFormat("Frame time: %02.02f ms", GetFrameTime()), 10, SCREEN_HEIGHT-30, 10, DARKGRAY);

        if (hoveredTile != NULL) {
          DrawText(TextFormat("Tile type: %s", hoveredTile->type->name), 10, 64, 10, DARKGRAY);
        }

        for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) {
          InventoryItem *item = player.inventory[i];
          int posX = 52*i+10;
          int posY = 10;
          int size = 48;
          DrawRectangle(posX, posY, size, size, YELLOW);
          if (item != NULL) {
            DrawTexturePro(
              *item->item.texture,
              (Rectangle){ 0, 0, (float)item->item.texture->width, (float)item->item.texture->height },   // full texture
              (Rectangle){ posX, posY, size, size },  // same size
              (Vector2){ 0, 0 },
              0.0f,
              WHITE
            );
            DrawText(TextFormat("%i", item->amount), posX+6, 36, 24, WHITE);
          }

          if (i == player.selectedInventoryItemIndex) {
            DrawRectangle(posX, posY, size, size, (Color){0,255,0,100});
          }
        }

        
        if (debugEnabled) {
          for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
            Tile tile = world[i];
            char text[32];
            snprintf(text, sizeof(text), "%.1f/%.1f", tile.breakTimeSecondsPassed, tile.type->breakTimeSeconds);

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
