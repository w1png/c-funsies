#include "player.h"
#include "crafting.h"
#include "objects.h"
#include "raylib.h"
#include "texture.h"
#include <limits.h>
#include <stdlib.h>

void InitPlayer(Player *player) {
  player->bounds = (Rectangle){
    0, // GetRandomValue(10, MAX_WORLD_SIZE*POINT_SIZE),
    0, // GetRandomValue(10, MAX_WORLD_SIZE*POINT_SIZE),
    POINT_SIZE * PLAYER_SCALE,
    POINT_SIZE * PLAYER_SCALE
  };
  player->selectedInventoryObjectIndex = 0;
  player->state = PS_NORMAL;
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

void DrawPlayer(Player *player, Vector2 deltas) {
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
