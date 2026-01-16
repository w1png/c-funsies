#include "const.h"
#include "objects.h"
#include "raylib.h"
#include <stdlib.h>
#include "world.h"

#define MAX_PONDS 10
#define MAX_POND_SIZE 150

/// Certified vibe code moment
void GeneratePond(Tile *world) {
    int target = GetRandomValue(MAX_POND_SIZE / 2, MAX_POND_SIZE);
    if (target < 4) target = 4;

    int cx = GetRandomValue(5, MAX_WORLD_SIZE - 6);
    int cy = GetRandomValue(5, MAX_WORLD_SIZE - 6);

    int placed = 0;

    for (int r = 1; r <= 12 && placed < target; r++) {
        for (int y = cy - r; y <= cy + r; y++) {
            if (y < 0 || y >= MAX_WORLD_SIZE) continue;

            for (int x = cx - r; x <= cx + r; x++) {
                if (x < 0 || x >= MAX_WORLD_SIZE) continue;

                int dx = x - cx;
                int dy = y - cy;

                if (dx*dx + dy*dy > r*r) continue;

                Tile *tile = &world[y * MAX_WORLD_SIZE + x];

                if (tile->object != objects.water) {
                    int chance = 100;

                    if (r >= 3) {
                        chance = 100 - (r - 2) * 12;
                        if (chance < 35) chance = 35;
                    }

                    if (GetRandomValue(0, 99) < chance) {
                        tile->object = objects.water;
                        placed++;

                        if (placed >= target) goto done;
                    }
                }
            }
        }
    }

done:
    TraceLog(LOG_INFO, "Pond center (%d,%d) → %d / %d water tiles", cx, cy, placed, target);
}

void GenerateWorld(Tile* world, Rectangle playerBounds) {
  TraceLog(LOG_INFO, "Generating world");
  for (int i = 0; i < MAX_WORLD_SIZE; i++) {
    TraceLog(LOG_INFO, "Generating row %i", i);
    for (int j = 0; j < MAX_WORLD_SIZE; j++) {
      Tile *tile = &world[i*(int)MAX_WORLD_SIZE+j];
      tile->bounds = (Rectangle){ i*POINT_SIZE, j*POINT_SIZE, POINT_SIZE, POINT_SIZE };
      tile->object = objects.grass;

      if (!CheckCollisionRecs(playerBounds, tile->bounds)) {
        if (GetRandomValue(0, 100) < 5) {
          tile->object = objects.tree;
        } else if (GetRandomValue(0, 100) < 5) {
          tile->object = objects.stone;
        }
      }

    }
  }

  for (int i = 0; i < GetRandomValue(1, MAX_PONDS); i++) {
    GeneratePond(world);
  }

  TraceLog(LOG_INFO, "World generated");
}
