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


typedef struct {
  bool* isExiting;
} PauseMenuData;

typedef struct {
  Player* player;
} PlayerInventoryData;

typedef struct {
  Tile* world;
  Player* player;
  Tile** hoveredTile;

  struct {
    bool showFPS;
    bool showHoveredTileType;
    bool showCollisions;
    bool showTileOrigin;
    bool showBreakTime;
    bool showTilePosition;
    bool showUILock;
  } features;
} DebugMenuData;

typedef struct {
  Player* player;
} InventoryMenuData;

typedef struct {
  Player* player;
  Tile* tile;
} ContainerMenuData;

typedef struct {
  UIScreen* pauseMenu;
  UIScreen* inventoryHUD;
  UIScreen* debugMenu;
  UIScreen* debugHUD;
  UIScreen* inventoryMenu;
  UIScreen* containerMenu;
} UIScreens;

extern UIScreens uiScreens;

#endif
