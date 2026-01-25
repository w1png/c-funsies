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
  bool isHovered;
} Tile;

void GenerateWorld(Tile* world, Rectangle playerBounds);
Rectangle GetTileDrawBounds(Tile* tile);
Rectangle GetTileCollisionBounds(Tile* tile);

void DrawWorldBackground(Tile* world);
void DrawWorldForeground(Tile* world);

#endif
