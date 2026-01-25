#include "player.h"
#include "camera.h"
#include "crafting.h"
#include "objects.h"
#include "raylib.h"
#include "raymath.h"
#include "texture.h"
#include "world.h"
#include <limits.h>
#include <stdlib.h>

void InitPlayer(Player *player) {
  player->bounds = (Rectangle){
    GetRandomValue(10, MAX_WORLD_SIZE*POINT_SIZE),
    GetRandomValue(10, MAX_WORLD_SIZE*POINT_SIZE),
    POINT_SIZE * PLAYER_SCALE,
    POINT_SIZE * PLAYER_SCALE
  };
  player->selectedInventoryObjectIndex = 0;
  player->state = PS_NORMAL;
}

float GetDistanceToTile(Player* player, Tile* tile) {
  return Vector2Distance(
    (Vector2){
      player->bounds.x + player->bounds.width / 2.0f,
      player->bounds.y + player->bounds.height / 2.0f
    }, 
    (Vector2){
      tile->bounds.x + tile->bounds.width / 2.0f,
      tile->bounds.y + tile->bounds.height / 2.0f
    }
  );
}

void HandleInventoryButtons(Player* player) {
  if (IsKeyPressed(KEY_ONE)) {
    player->selectedInventoryObjectIndex = 0;
  }
  if (IsKeyPressed(KEY_TWO)) {
    player->selectedInventoryObjectIndex = 1;
  }
  if (IsKeyPressed(KEY_THREE)) {
    player->selectedInventoryObjectIndex = 2;
  }
  if (IsKeyPressed(KEY_FOUR)) {
    player->selectedInventoryObjectIndex = 3;
  }
  if (IsKeyPressed(KEY_FIVE)) {
    player->selectedInventoryObjectIndex = 4;
  }
}

bool CanBreakTile(Player* player, Tile* tile) {
  return (
    IsHoveringTile(player, tile) &&
    HAS_TAG(tile->object, TAG_BREAKABLE)
  );
}

bool HandleBreakTile(Player* player, Tile* tile) {
  tile->breakTimeSecondsPassed += GetFrameTime();
  if (tile->breakTimeSecondsPassed >= tile->object->breakTimeSeconds && tile->object->onBreak != NULL) {
    tile->object->onBreak(tile->object, tile, player);

    return true;
  }

  return false;
}

bool CanPlaceTile(Player* player, Tile* tile) {
  InventoryObject *inventoryObject = player->inventory[player->selectedInventoryObjectIndex];
  return (
    HAS_TAG(tile->object, TAG_EMPTY) &&
    inventoryObject != NULL &&
    HAS_TAG(inventoryObject->object, TAG_PLACEABLE) && 
    !CheckPlayerCollision(player, tile)
  );
}

bool CanInteractTile(Player* player, Tile* tile) {
  return (
    HAS_TAG(tile->object, TAG_INTERACTABLE) && tile->object->onClick != NULL
  );
}

bool HandlePlaceTile(Player* player, Tile* tile) {
  InventoryObject *inventoryObject = player->inventory[player->selectedInventoryObjectIndex];
  if (inventoryObject->object->onPlace != NULL) {
    inventoryObject->object->onPlace(inventoryObject->object, tile, player);
    return true;
  }

  return false;
}

bool IsHoveringTile(Player* player, Tile* tile) {
  return GetDistanceToTile(player, tile) <= (INTERACT_DISTANCE * POINT_SIZE) &&
    CheckCollisionPointRec(GetScreenToWorld2D(GetMousePosition(), camera), tile->bounds);
}

bool CheckPlayerCollision(Player* player, Tile* tile) {
  return CheckCollisionRecs(player->bounds, GetTileCollisionBounds(tile));
}

void HandlePlayerCollision(Player* player, Tile* tile) {
  bool hasCollidedWall = false;
}

void HandlePlayerMovement(Player* player) {
  player->movementDeltas.x = 0;
  player->movementDeltas.y = 0;
  if (IsKeyDown(KEY_A)) {
    player->movementDeltas.x -= PLAYER_SPEED;
  }
  if (IsKeyDown(KEY_D)) {
    player->movementDeltas.x += PLAYER_SPEED;
  }
  if (IsKeyDown(KEY_S)) {
    player->movementDeltas.y += PLAYER_SPEED;
  }
  if (IsKeyDown(KEY_W)) {
    player->movementDeltas.y -= PLAYER_SPEED;
  }
  float runingMultiplier = (IsKeyDown(KEY_LEFT_SHIFT)) ? RUN_MULTIPLIER : 1.0f;

  player->bounds.x += player->movementDeltas.x * GetFrameTime() * runingMultiplier;
  player->bounds.y += player->movementDeltas.y * GetFrameTime() * runingMultiplier;
}

void AddToPlayerInventory(Object *object, int amount, Player *player) {
  TraceLog(LOG_INFO, "Adding %d of %s to player inventory", amount, object->name);
  bool hasObject = false;
  int firstFreeSlot = -1;
  for (int i = 0; i < MAX_INVENTORY_OBJECTS; i++) {
    InventoryObject *inventoryObject = player->inventory[i];
    if (inventoryObject == NULL) {
      firstFreeSlot = i;
      break;
    }

    if (inventoryObject->object->id == object->id) {
      inventoryObject->amount += amount;
      hasObject = true;
    }
  }

  if (!hasObject) {
    if (firstFreeSlot == -1) {
      TraceLog(LOG_ERROR, "No free inventory slot");
      return;
    }

    player->inventory[firstFreeSlot] = (InventoryObject*)malloc(sizeof(InventoryObject));
    player->inventory[firstFreeSlot]->object = object;
    player->inventory[firstFreeSlot]->amount = amount;
  }

  TraceLog(LOG_INFO, "Added %d of %s to player inventory", amount, object->name);
}

void HandleUpdateInventory(Player *player) {
  for (int i = 0; i < MAX_INVENTORY_OBJECTS; i++) {
    InventoryObject *inventoryObject = player->inventory[i];
    if (inventoryObject != NULL && inventoryObject->amount == 0) {
      player->inventory[i] = NULL;
    }
  }
}

void DrawPlayer(Player *player) {
  DrawTexturePro(
    textures.player, 
    (Rectangle){0, 0, (float)textures.player.width, (float)textures.player.height}, 
    (Rectangle){player->bounds.x, player->bounds.y, (float)POINT_SIZE, (float)POINT_SIZE},
    (Vector2){0,0}, 
    0.0f, WHITE
  );
}

bool HasInventoryObject(Player *player, Object *object, int amount) { 
  if (amount <= 0) return true;

  for (int i = 0; i < MAX_INVENTORY_OBJECTS; i++) {
    InventoryObject *inventoryObject = player->inventory[i];
    if (inventoryObject == NULL) continue;
    if (inventoryObject->object != object) continue;
    return inventoryObject->amount >= amount;
  }
  return false;
}

int GetMaxCraftAmount(CraftingRecipe *recipe, Player *player) {
  if (recipe->ingredientsCount == 0) return 0;

  int amount = INT_MAX;

  for (int i = 0; i < recipe->ingredientsCount; i++) {
      const CraftingIngredient *ingredient = &recipe->ingredients[i];
      if (ingredient->amount <= 0) continue;

      int have = 0;
      for (int s = 0; s < MAX_INVENTORY_OBJECTS; s++) {
          InventoryObject *slot = player->inventory[s];
          if (slot == NULL) continue;
          if (slot->object == ingredient->object) {
              have += slot->amount;
          }
      }

      int can_make = have / ingredient->amount;
      if (can_make < amount) {
          amount = can_make;
      }
  }

  return amount;
}

int CraftRecipe(CraftingRecipe *recipe, int amount, Player *player) {
  TraceLog(LOG_INFO, "Crafting recipe %s", recipe->name);
  int maxCraftAmount = GetMaxCraftAmount(recipe, player);
  if (!maxCraftAmount) {
    TraceLog(LOG_ERROR, "Not enough ingredients");
    return -1;
  };

  for (int i = 0; i < recipe->ingredientsCount; i++) {
    CraftingIngredient *ingredient = &recipe->ingredients[i];
    for (int j = 0; j < MAX_INVENTORY_OBJECTS; j++) {
      InventoryObject *inventoryObject = player->inventory[j];
      if (inventoryObject == NULL) continue;
      if (inventoryObject->object != ingredient->object) continue;
      inventoryObject->amount -= ingredient->amount*amount;
    }
  }

  TraceLog(LOG_INFO, "Crafted recipe %s", recipe->name);
  AddToPlayerInventory(recipe->result, recipe->resultAmount*amount, player);
  HandleUpdateInventory(player);
  return 0;
}
