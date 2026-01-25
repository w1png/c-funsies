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

int OnBreakParticles(Object* object, Tile* tile) {
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

int OnBreakTree(Object* object, Tile* tile, Player* player) {
  AddToPlayerInventory(objects.wood, GetRandomValue(1, 3), player);
  tile->object = objects.grass;
  // OnBreakParticles(object, data);

  return 0;
}

int OnBreakGeneric(Object* object, Tile* tile, Player* player) {
  Object *obj = (Object*)object;

  AddToPlayerInventory(obj, 1, player);
  tile->object = objects.grass;
  tile->data = NULL;

  // OnBreakParticles(object, data);

  return 0;
}


int OnBreakContainer(Object* object, Tile* tile, Player* player) {
  if (tile->data != NULL) {
    free(tile->data);
  }
  // TODO: implement dropping items
  return OnBreakGeneric(object, tile, player);
}

int OnClickContainer(Object* object, Tile* tile, Player* player) {
  TraceLog(LOG_INFO, "Opening container");
  ContainerMenuData *d = malloc(sizeof(ContainerMenuData));
  d->tile = tile;
  d->player = (Player*)player;
  uiScreens.containerMenu->data = d;
  SetScreenOpen(uiScreens.containerMenu, true);

  return 0;
}

int OnPlaceGeneric(Object* object, Tile* tile, Player* player) {
  InventoryObject *inventoryObject = player->inventory[player->selectedInventoryObjectIndex];
  if (inventoryObject == NULL) {
    return -1;
  };

  tile->object = inventoryObject->object;
  inventoryObject->amount--;
  HandleUpdateInventory(player);

  return 0;
}

int OnPlaceContainer(Object* object, Tile* tile, Player* player) {
  tile->data = malloc(sizeof(ContainerObject)*CHEST_SIZE);

  return OnPlaceGeneric(object, tile, player);
}

void RegisterAllObjects() {
  Rectangle singleTileBounds = {.x = 0, .y = 0, .width = 1, .height = 1};

  TraceLog(LOG_INFO, "===Registering objects===");
  objects.tree = RegisterObject("tree", &textures.tree);
  objects.tree->tags = TAG_BLOCKING | TAG_BREAKABLE;
  objects.tree->breakTimeSeconds = 1;
  objects.tree->onBreak = (ObjectCb)OnBreakTree;
  objects.tree->drawBounds = (Rectangle){
    .x = -1,
    .y = -2,
    .width = 3,
    .height = 3,
  };
  objects.tree->collisionBounds = (Rectangle){
    .x = -0.5,
    .y = 0,
    .width = 2,
    .height = 1,
  };

  objects.wood = RegisterObject("wood", &textures.wood);
  objects.wood->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE;
  objects.wood->breakTimeSeconds = 0.5;
  objects.wood->onBreak = (ObjectCb)OnBreakGeneric;
  objects.wood->onPlace = (ObjectCb)OnPlaceGeneric;
  objects.wood->drawBounds = singleTileBounds;
  objects.wood->collisionBounds = singleTileBounds;

  objects.stone = RegisterObject("stone", &textures.stone);
  objects.stone->tags = TAG_BLOCKING | TAG_PLACEABLE | TAG_BREAKABLE;
  objects.stone->breakTimeSeconds = 2;
  objects.stone->onBreak = (ObjectCb)OnBreakGeneric;
  objects.stone->onPlace = (ObjectCb)OnPlaceGeneric;
  objects.stone->drawBounds = singleTileBounds;
  objects.stone->collisionBounds = singleTileBounds;

  objects.grass = RegisterObject("grass", &textures.grass);
  objects.grass->tags = TAG_EMPTY | TAG_BACKGROUND;
  objects.grass->drawBounds = singleTileBounds;

  objects.water = RegisterObject("water", &textures.water);
  objects.water->tags = TAG_BLOCKING | TAG_FISHING_SPOT | TAG_BACKGROUND;
  objects.water->drawBounds = singleTileBounds;

  objects.fence = RegisterObject("fence", &textures.placeholder);
  objects.fence->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE;
  objects.fence->breakTimeSeconds = 0.5;
  objects.fence->onBreak = (ObjectCb)OnBreakGeneric;
  objects.fence->onPlace = (ObjectCb)OnPlaceGeneric;
  objects.fence->drawBounds = singleTileBounds;

  objects.chest = RegisterObject("chest", &textures.chest);
  objects.chest->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE | TAG_CONTAINER | TAG_INTERACTABLE;
  objects.chest->breakTimeSeconds = 0.5;
  objects.chest->onBreak = (ObjectCb)OnBreakContainer;
  objects.chest->onClick = (ObjectCb)OnClickContainer;
  objects.chest->onPlace = (ObjectCb)OnPlaceContainer;
  objects.chest->drawBounds = singleTileBounds;

  TraceLog(LOG_INFO, "===Objects registered===");
}

