#ifndef UI_H
#define UI_H

#include "player.h"
#include "world.h"
#include <stdbool.h>

#define BUTTON_HEIGHT 48
#define PADDING 12
#define GAP 8

typedef struct {
  const char* name;
  void (*handle)(void* screen);
  bool isOpen;

  void* data;
} UIScreen;

void RegisterAllUIScreens();
void HandleAllUIUpdates();

extern UIScreen* pauseMenu;
typedef struct {
  bool* isExiting;
} PauseMenuData;

extern UIScreen* playerInventoryMenu;
typedef struct {
  Player* player;
} PlayerInventoryData;

extern UIScreen* debugMenu;
typedef struct {
  Tile* world;
  Player* player;
  Tile** hoveredTile;
  Camera2D* camera;

  struct {
    bool showFPS;
    bool showHoveredTileType;
    bool showCollisions;
    bool showBreakTime;
  } features;
} DebugMenuData;

#endif
