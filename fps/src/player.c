#include "player.h"
#include "objects.h"
#include "texture.h"
#include <stdlib.h>

void InitPlayer(Player *player) {
  player->bounds = (Rectangle){
    0.0f,
    0.0f,
    POINT_SIZE * PLAYER_SCALE,
    POINT_SIZE * PLAYER_SCALE
  };
  player->selectedInventoryObjectIndex = 0;
}

void AddToPlayerInventory(Object *object, int amount, Player *player) {
  bool hasObject = false;
  int firstFreeSlot = -1;
  for (int i = 0; i < MAX_INVENTORY_OBJECTS; i++) {
    InventoryObject *inventoryObject = player->inventory[i];
    if (inventoryObject == NULL) {
      firstFreeSlot = i;
      break;
    }

    if (inventoryObject->object->name == object->name) {
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
    Rectangle source = { 0.0f, 0.0f, (float)textures.player.width * (deltas.x > 0 ? 1 : -1), (float)textures.player.height };
    DrawTextureRec(textures.player, source, (Vector2){ player->bounds.x, player->bounds.y }, WHITE);
}

void DrawInventory(Player *player) {
  for (int i = 0; i < MAX_INVENTORY_OBJECTS; i++) {
    InventoryObject *object = player->inventory[i];
    int posX = 52*i+10;
    int posY = 10;
    int size = 48;
    DrawRectangle(posX, posY, size, size, YELLOW);
    if (object != NULL) {
      DrawTexturePro(
        *object->object->texture,
        (Rectangle){ 0, 0, (float)object->object->texture->width, (float)object->object->texture->height },   // full texture
        (Rectangle){ posX, posY, size, size },  // same size
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
      );
      DrawText(TextFormat("%i", object->amount), posX+6, 36, 24, WHITE);
    }

    if (i == player->selectedInventoryObjectIndex) {
      DrawRectangle(posX, posY, size, size, (Color){0,255,0,100});
    }
  }
}
