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

UIScreen* pauseMenu;
typedef struct {
  bool* isExiting;
} PauseMenuData;

UIScreen* inventoryHUD;
typedef struct {
  Player* player;
} PlayerInventoryData;

UIScreen* debugMenu;

UIScreen* debugHUD;
typedef struct {
  Tile* world;
  Player* player;
  Tile** hoveredTile;

  struct {
    bool showFPS;
    bool showHoveredTileType;
    bool showCollisions;
    bool showBreakTime;
    bool showUILock;
  } features;
} DebugMenuData;

UIScreen* inventoryMenu;
typedef struct {
  Player* player;
} InventoryMenuData;

UIScreen* containerMenu;
typedef struct {
  Player* player;
  Tile* tile;
} ContainerMenuData;

#endif
