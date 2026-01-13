#include "const.h"
#include "objects.h"
#include "raylib.h"
#include "world.h"

void GenerateWorld(Tile *world) {
  TraceLog(LOG_INFO, "Generating world");
  for (int i = 0; i < MAX_WORLD_SIZE; i++) {
    TraceLog(LOG_INFO, "Generating row %i", i);
    for (int j = 0; j < MAX_WORLD_SIZE; j++) {
      Tile *tile = &world[i*(int)MAX_WORLD_SIZE+j];
      tile->bounds = (Rectangle){ i*TILE_SIZE.x, j*TILE_SIZE.y, TILE_SIZE.x, TILE_SIZE.y };
      tile->object = GRASS;

      if (GetRandomValue(0, 100) < 5) {
        tile->object = TREE;
      } else if (GetRandomValue(0, 100) < 5) {
        tile->object = STONE;
      }
    }
  }
  TraceLog(LOG_INFO, "World generated");
}
