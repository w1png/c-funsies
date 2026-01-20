#include "objects.h"
#include "particle.h"
#include "player.h"
#include "texture.h"
#include "raylib.h"
#include "ui.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>

Objects objects;

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

int OnBreakParticles(void* object, void *data) {
  Object *obj = (Object*)object;
  Tile *tile = (Tile*)data;
  Vector2 pos = {
    tile->bounds.x + tile->bounds.width / 2.0f,
    tile->bounds.y + tile->bounds.height / 2.0f
  };
  if (RegisterParticleEmitter(pos, emitterConfigs.breakBlock) != 0) {
    TraceLog(LOG_ERROR, "Failed to register particle emitter inside OnBreakParticles");
    return -1;
  }

  return 0;
}

int OnBreakTree(void* object, void *data, void *player) {
  Tile *tile = (Tile*)data;
  AddToPlayerInventory(objects.wood, GetRandomValue(1, 3), player);
  tile->object = objects.grass;
  // OnBreakParticles(object, data);

  return 0;
}

int OnBreakGeneric(void* object, void *data, void *player) {
  Object *obj = (Object*)object;

  Tile *tile = (Tile*)data;
  AddToPlayerInventory(obj, 1, player);
  tile->object = objects.grass;
  tile->data = NULL;

  // OnBreakParticles(object, data);

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
  uiScreens.containerMenu->data = d;
  SetScreenOpen(uiScreens.containerMenu, true);

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
  TraceLog(LOG_INFO, "===Registering objects===");
  objects.tree = RegisterObject("tree", &textures.tree);
  objects.tree->tags = TAG_BLOCKING | TAG_BREAKABLE;
  objects.tree->breakTimeSeconds = 1;
  objects.tree->onBreak = OnBreakTree;

  objects.wood = RegisterObject("wood", &textures.wood);
  objects.wood->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE;
  objects.wood->breakTimeSeconds = 0.5;
  objects.wood->onBreak = OnBreakGeneric;
  objects.wood->onPlace = OnPlaceGeneric;

  objects.stone = RegisterObject("stone", &textures.stone);
  objects.stone->tags = TAG_BLOCKING | TAG_PLACEABLE | TAG_BREAKABLE;
  objects.stone->breakTimeSeconds = 2;
  objects.stone->onBreak = OnBreakGeneric;
  objects.stone->onPlace = OnPlaceGeneric;

  objects.grass = RegisterObject("grass", &textures.grass);
  objects.grass->tags = TAG_EMPTY | TAG_BACKGROUND;

  objects.water = RegisterObject("water", &textures.water);
  objects.water->tags = TAG_BLOCKING | TAG_FISHING_SPOT | TAG_BACKGROUND;

  objects.fence = RegisterObject("fence", &textures.placeholder);
  objects.fence->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE;
  objects.fence->breakTimeSeconds = 0.5;
  objects.fence->onBreak = OnBreakGeneric;
  objects.fence->onPlace = OnPlaceGeneric;

  objects.chest = RegisterObject("chest", &textures.chest);
  objects.chest->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE | TAG_CONTAINER | TAG_INTERACTABLE;
  objects.chest->breakTimeSeconds = 0.5;
  objects.chest->onBreak = OnBreakContainer;
  objects.chest->onClick = OnClickContainer;
  objects.chest->onPlace = OnPlaceContainer;

  TraceLog(LOG_INFO, "===Objects registered===");
}

