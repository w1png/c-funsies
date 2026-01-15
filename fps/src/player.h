#ifndef PLAYER_H
#define PLAYER_H

#include "crafting.h"
#include "raylib.h"
#include "const.h"
#include "objects.h"

typedef struct {
  Object *object;
  int amount;
} InventoryObject;

typedef enum {
  PS_NORMAL,
  PS_FISHING,
} PlayerState;

typedef struct {
  Rectangle bounds;
  InventoryObject* inventory[MAX_INVENTORY_OBJECTS];
  int selectedInventoryObjectIndex;
  PlayerState state;
} Player;

void InitPlayer(Player *player);

void AddToPlayerInventory(Object *item, int amount, Player *player);
void HandleUpdateInventory(Player *player);
int GetMaxCraftAmount(CraftingRecipe *recipe, Player *player);
bool HasInventoryObject(Player *player, Object *object, int amount);
int CraftRecipe(CraftingRecipe *recipe, int amount, Player *player);

void DrawPlayer(Player *player, Vector2 deltas);

#endif
