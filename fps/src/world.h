#ifndef WORLD_H
#define WORLD_H

#include "raylib.h"
#include "const.h"
#include "objects.h"

typedef struct {
  Rectangle bounds;
  Object *object;

  float breakTimeSecondsPassed;
} Tile;

void GenerateWorld(Tile *world);

#endif
