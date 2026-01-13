#include "objects.h"
#include "player.h"
#include "texture.h"
#include "raylib.h"
#include "world.h"

Object objectDefs[128];
int objectDefsCount = 0;

Object *RegisterObject(const char* name, Texture2D *texture) {
  TraceLog(LOG_INFO, "Registering object %s", name);
  Object *object = &objectDefs[objectDefsCount++];
  object->id = objectDefsCount;
  object->name = name;
  object->texture = texture;
  TraceLog(LOG_INFO, "Object %s registered", name);
  return object;
}

void* OnBreakTree(void* object, void *data, void *player) {
  Tile *tile = (Tile*)data;
  AddToPlayerInventory(WOOD, GetRandomValue(1, 3), player);
  tile->object = GRASS;

  return 0;
}

void* OnBreakGeneric(void* object, void *data, void *player) {
  Object *obj = (Object*)object;

  Tile *tile = (Tile*)data;
  AddToPlayerInventory(obj, 1, player);
  tile->object = GRASS;

  return 0;
}

void RegisterObjects() {
  TraceLog(LOG_INFO, "Registering objects");
  TREE = RegisterObject("tree", &textures.tree);
  TREE->tags = TAG_BLOCKING | TAG_BREAKABLE;
  TREE->breakTimeSeconds = 1;
  TREE->onBreak = OnBreakTree;

  WOOD = RegisterObject("wood", &textures.wood);
  WOOD->tags = TAG_BLOCKING | TAG_BREAKABLE | TAG_PLACEABLE;
  WOOD->breakTimeSeconds = 0.5;
  WOOD->onBreak = OnBreakGeneric;

  STONE = RegisterObject("stone", &textures.stone);
  STONE->tags = TAG_BLOCKING | TAG_PLACEABLE | TAG_BREAKABLE;
  STONE->breakTimeSeconds = 2;
  STONE->onBreak = OnBreakGeneric;

  GRASS = RegisterObject("grass", &textures.grass);
  GRASS->tags = TAG_EMPTY;

  TraceLog(LOG_INFO, "Objects registered");
}

