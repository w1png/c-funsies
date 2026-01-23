#ifndef WORLD_H
#define WORLD_H

#include "raylib.h"
#include "const.h"
#include "objects.h"

typedef struct {
  Rectangle bounds;
  Object *object;

  float breakTimeSecondsPassed;
  void* data;
} Tile;

void GenerateWorld(Tile* world, Rectangle playerBounds);
Rectangle GetTileDrawBounds(Tile* tile);
Rectangle GetTileCollisionBounds(Tile* tile);

#endif
