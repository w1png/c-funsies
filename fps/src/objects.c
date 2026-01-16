#include "objects.h"
#include "player.h"
#include "texture.h"
#include "raylib.h"
#include "ui.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>

Object objectDefs[MAX_OBJECTS];
int objectDefsCount = 0;

Object *RegisterObject(const char* name, Texture2D *texture) {
  if (objectDefsCount >= MAX_OBJECTS) {
    TraceLog(LOG_ERROR, "Too many objects registered");
    exit(1);
  }

  TraceLog(LOG_INFO, "Registering object %s", name);
  Object *object = &objectDefs[objectDefsCount++];
  object->id = objectDefsCount;
  object->name = name;
  object->texture = texture;
  TraceLog(LOG_INFO, "Object %s registered", name);
  return object;
}

int OnBreakTree(void* object, void *data, void *player) {
  Tile *tile = (Tile*)data;
  AddToPlayerInventory(WOOD, GetRandomValue(1, 3), player);
  tile->object = GRASS;

  return 0;
}

int OnBreakGeneric(void* object, void *data, void *player) {
  Object *obj = (Object*)object;

  Tile *tile = (Tile*)data;
  AddToPlayerInventory(obj, 1, player);
  tile->object = GRASS;
  tile->data = NULL;

  return 0;
}


int OnBreakContainer(void* object, void* data, void *player) {
  // TODO: implement dropping items
  return OnBreakGeneric(object, data, player);
}

int OnClickContainer(void* object, void* data, void *player) {
  TraceLog(LOG_INFO, "Opening container");
  ContainerMenuData *d = malloc(sizeof(ContainerMenuData));
  d->tile = (Tile*)data;
  d->player = (Player*)player;
  containerMenu->data = d;
  SetScreenOpen(containerMenu, true);

  return 0;
}

int OnPlaceGeneric(void* object, void* data, void *player) {
  Object *obj = (Object*)object;
  Tile* tile = (Tile*)data;
  Player* pl = (Player*)player;

  InventoryObject *inventoryObject = pl->inventory[pl->selectedInventoryObjectIndex];
  if (inventoryObject == NULL) {
    return -1;
  };

  tile->object = inventoryObject->object;
  inventoryObject->amount--;
  HandleUpdateInventory(player);

  return 0;
}

int OnPlaceContainer(void* object, void* data, void *player) {
  Tile* tile = (Tile*)data;

  tile->data = malloc(sizeof(ContainerObject)*CHEST_SIZE);

  return OnPlaceGeneric(object, data, player);
}

void RegisterAllObjects() {
  TraceLog(LOG_INFO, "Registering objects");
  TREE = RegisterObject("tree", &textures.tree);
  TREE->tags = TAG_BLOCKING | TAG_BREAKABLE;
  TREE->breakTimeSeconds = 1;
  TREE->onBreak = OnBreakTree;

  WOOD = RegisterObject("wood", &textures.wood);
  WOOD->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE;
  WOOD->breakTimeSeconds = 0.5;
  WOOD->onBreak = OnBreakGeneric;
  WOOD->onPlace = OnPlaceGeneric;

  STONE = RegisterObject("stone", &textures.stone);
  STONE->tags = TAG_BLOCKING | TAG_PLACEABLE | TAG_BREAKABLE;
  STONE->breakTimeSeconds = 2;
  STONE->onBreak = OnBreakGeneric;
  STONE->onPlace = OnPlaceGeneric;

  GRASS = RegisterObject("grass", &textures.grass);
  GRASS->tags = TAG_EMPTY | TAG_BACKGROUND;

  WATER = RegisterObject("water", &textures.water);
  WATER->tags = TAG_BLOCKING | TAG_FISHING_SPOT | TAG_BACKGROUND;

  FENCE = RegisterObject("fence", &textures.placeholder);
  FENCE->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE;
  FENCE->breakTimeSeconds = 0.5;
  FENCE->onBreak = OnBreakGeneric;
  FENCE->onPlace = OnPlaceGeneric;

  CHEST = RegisterObject("chest", &textures.chest);
  CHEST->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE | TAG_CONTAINER | TAG_INTERACTABLE;
  CHEST->breakTimeSeconds = 0.5;
  CHEST->onBreak = OnBreakContainer;
  CHEST->onClick = OnClickContainer;
  CHEST->onPlace = OnPlaceContainer;

  TraceLog(LOG_INFO, "Objects registered");
}

