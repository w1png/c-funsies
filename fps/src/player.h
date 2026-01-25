#ifndef PLAYER_H
#define PLAYER_H

#include "crafting.h"
#include "raylib.h"
#include "const.h"
#include "objects.h"
#include "world.h"

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

  Vector2 movementDeltas;
} Player;

void InitPlayer(Player *player);

bool CanInteractTile(Player* player, Tile* tile);
bool CanBreakTile(Player* player, Tile* tile);
bool HandleBreakTile(Player* player, Tile* tile);
bool CanPlaceTile(Player* player, Tile* tile);
bool HandlePlaceTile(Player* player, Tile* tile);

void HandleInventoryButtons(Player* player);
float GetDistanceToTile(Player* player, Tile* tile);

bool IsHoveringTile(Player* player, Tile* tile);
void HandlePlayerMovement(Player* player);
bool CheckPlayerCollision(Player* player, Tile* tile);
void HandlePlayerCollision(Player* player, Tile* tile);

void AddToPlayerInventory(Object *item, int amount, Player *player);
void HandleUpdateInventory(Player *player);
int GetMaxCraftAmount(CraftingRecipe *recipe, Player *player);
bool HasInventoryObject(Player *player, Object *object, int amount);
int CraftRecipe(CraftingRecipe *recipe, int amount, Player *player);

void DrawPlayer(Player *player);

#endif
