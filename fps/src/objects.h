#ifndef OBJECTS_H
#define OBJECTS_H

#include "raylib.h"
#include <stdint.h>

typedef enum {
  TAG_BLOCKING     = 1 << 0,
  TAG_BREAKABLE    = 1 << 1,
  TAG_PLACEABLE    = 1 << 2,
  TAG_EMPTY        = 1 << 3,
  TAG_BACKGROUND   = 1 << 4,
  TAG_FISHING_SPOT = 1 << 5,
  TAG_CONTAINER    = 1 << 6,
  TAG_INTERACTABLE = 1 << 7,
} ObjectTags;

typedef struct {
  int id;
  const char *name;
  uint32_t tags;
  Texture2D *texture;
  void* data;

  float breakTimeSeconds;
  
  int (*onBreak)(void* object, void *data, void *player);
  int (*onClick)(void* object, void *data, void *player);
  int (*onPlace)(void* object, void *data, void *player);
  int (*onStep)(void* object, void *data, void *player);
} Object;

typedef struct {
  Object *object;
  int amount;
} ContainerObject;

Object *TREE;
Object *WOOD;
Object *STONE;
Object *GRASS;
Object *WATER;
Object *FENCE;
Object *CHEST;

Object *RegisterObject(const char* name, Texture2D *texture);
void RegisterAllObjects();

#endif
