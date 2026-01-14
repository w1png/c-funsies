#ifndef UI_H
#define UI_H

#include "player.h"
#include "world.h"
#include <stdbool.h>

typedef struct {
  int id;
  const char* name;
  void (*handle)(void* screen);
  bool isOpen;
  bool isBlocking;

  void* data;
} UIScreen;

void RegisterAllUIScreens();
void HandleAllUIUpdates();
void SetScreenOpen(UIScreen* us, bool isOpen);

UIScreen* GetUILock();

extern UIScreen* pauseMenu;
typedef struct {
  bool* isExiting;
} PauseMenuData;

extern UIScreen* playerInventoryMenu;
typedef struct {
  Player* player;
} PlayerInventoryData;

extern UIScreen* debugMenu;

extern UIScreen* debugInfo;
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
    bool showUILock;
  } features;
} DebugMenuData;

#endif
