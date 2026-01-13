#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "const.h"
#include "objects.h"

typedef struct {
  Object *object;
  int amount;
} InventoryObject;

typedef struct {
  Rectangle bounds;
  InventoryObject* inventory[MAX_INVENTORY_OBJECTS];
  int selectedInventoryObjectIndex;
} Player;

void InitPlayer(Player *player);

void AddToPlayerInventory(Object *item, int amount, Player *player);
void HandleUpdateInventory(Player *player);

void DrawPlayer(Player *player, Vector2 deltas);
void DrawInventory(Player *player);

#endif
