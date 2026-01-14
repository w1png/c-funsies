#include "ui.h"
#include "objects.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "lib/raygui.h"
#include "const.h"
#include <stdlib.h>

UIScreen* pauseMenu;
UIScreen* playerInventoryMenu;
UIScreen* debugMenu;

UIScreen ui_screens[MAX_UI_SCREENS];
int ui_screen_count = 0;


UIScreen* RegisterUIScreen(const char* name) {
  UIScreen* us = &ui_screens[ui_screen_count++];
  us->name = name;
  us->isOpen = false;
  return us;
} 

void HandleAllUIUpdates() {
  UIScreen* us;
  for (int i = 0; i < ui_screen_count; i++) {
    us = &ui_screens[i];
    us->handle(us);
  }
}

void DrawBackground() {
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0,0,0,150});
}

void HandlePauseMenu(void* screen) {
  UIScreen* us = screen;
  if (!us->isOpen) {
    return;
  }
  PauseMenuData* data = us->data;

  float width = 400.0f;
  float height = PADDING*2+BUTTON_HEIGHT*2+GAP*1;
  Vector2 position = (Vector2){GetScreenWidth()/2.0f - width/2.0f, GetScreenHeight()/2.0f - height/2.0f};

  DrawBackground();

  DrawRectangle(position.x, position.y, width, height, WHITE);
  if (GuiButton((Rectangle){position.x + PADDING, position.y + PADDING, width - PADDING*2, BUTTON_HEIGHT}, "Resume")) {
    us->isOpen = false;
  }

  if (GuiButton((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*1 + GAP*1, width - PADDING*2, BUTTON_HEIGHT}, "Quit")) {
    us->isOpen = false;
    *data->isExiting = true;
  }
}

void HandlePlayerInventoryMenu(void* screen) {
  UIScreen* us = screen;
  PlayerInventoryData* data = us->data;
  Player* player = data->player;

  for (int i = 0; i < MAX_INVENTORY_OBJECTS; i++) {
    InventoryObject *object = player->inventory[i];
    int posX = PADDING+BUTTON_HEIGHT*i+GAP/2*i;
    int posY = PADDING;
    if (GuiButton((Rectangle){posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT}, "")) {
      player->selectedInventoryObjectIndex = i;
    }
    if (object != NULL) {
      DrawTexturePro(
        *object->object->texture,
        (Rectangle){ 0, 0, (float)object->object->texture->width, (float)object->object->texture->height },
        (Rectangle){ posX+2, posY+2, BUTTON_HEIGHT-4, BUTTON_HEIGHT-4},
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
      );
      DrawText(TextFormat("%i", object->amount), posX+PADDING/4, posY, 24, WHITE);
    }

    if (i == player->selectedInventoryObjectIndex) {
      DrawRectangle(posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT, (Color){0,255,0,100});
    }
}
}

void HandleDebugMenu(void* screen) {
  UIScreen* us = screen;
  DebugMenuData* data = us->data;

  int totalButtons = 5;
  float width = 400.0f;
  float height = PADDING*2+BUTTON_HEIGHT*totalButtons+GAP*(totalButtons-1);
  Vector2 position = (Vector2){GetScreenWidth()/2.0f - width/2.0f, GetScreenHeight()/2.0f - height/2.0f};

  if (data->features.showFPS) {
      const char *fpsText = 0;
      if (TARGET_FPS <= 0) fpsText = TextFormat("FPS: unlimited (%i)", GetFPS());
      else fpsText = TextFormat("FPS: %i (target: %i)", GetFPS(), TARGET_FPS);
      DrawText(fpsText, PADDING, GetScreenHeight()-PADDING, 16, PINK);
      DrawText(TextFormat("Frame time: %02.02f ms", GetFrameTime()), PADDING, GetScreenHeight()-PADDING-GAP, 16, PINK);
  }

  if (data->features.showHoveredTileType) {
    if (*data->hoveredTile != NULL) {
      DrawText(TextFormat("Hovered tile type: %s", (*data->hoveredTile)->object->name), PADDING, GetScreenHeight()-PADDING-GAP*2-16, 16, PINK);
    }
  }

  for (int i = 0; i < MAX_WORLD_SIZE*MAX_WORLD_SIZE; i++) {
    Tile tile = data->world[i];
    if (data->features.showCollisions) {
      if (HAS_TAG(tile.object, TAG_BLOCKING)) {
        Vector2 screenPos = GetWorldToScreen2D(
          (Vector2){
            tile.bounds.x,
            tile.bounds.y,
          }, 
          *data->camera
        );

        DrawRectangleLinesEx(
          (Rectangle){
            screenPos.x,
            screenPos.y,
            tile.bounds.width/2,
            tile.bounds.height/2,
          },
          1,
          PINK
        );
      }
    }

    if (data->features.showBreakTime) {
      if (HAS_TAG(tile.object, TAG_BREAKABLE)) {
        DrawTextEx(
          GetFontDefault(),
          TextFormat("%.1f/%.1f", tile.breakTimeSecondsPassed, tile.object->breakTimeSeconds),
          GetWorldToScreen2D(
            (Vector2){
              tile.bounds.x + tile.bounds.width / 2.0f -20,
              tile.bounds.y + tile.bounds.height / 2.0f -20
            }, 
            *data->camera
          ),
          6,
          1,
          WHITE
        );
      }
    }
  }

  if (us->isOpen) {
    DrawBackground();
    DrawRectangle(position.x, position.y, width, height, WHITE);
    GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s FPS", data->features.showFPS ? "Hide" : "Show"), &data->features.showFPS);
    GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT + GAP, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s hovered tile type", data->features.showHoveredTileType ? "Hide" : "Show"), &data->features.showHoveredTileType);
    GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*2 + GAP*2, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s collisions", data->features.showCollisions ? "Hide" : "Show"), &data->features.showCollisions);
    GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*3 + GAP*3, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s break time", data->features.showBreakTime ? "Hide" : "Show"), &data->features.showBreakTime);
    if (GuiButton((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*4 + GAP*4, width - PADDING*2, BUTTON_HEIGHT}, "Close")) {
      us->isOpen = false;
    }
  }

}

void RegisterAllUIScreens() {
  playerInventoryMenu = RegisterUIScreen("Player Inventory");
  playerInventoryMenu->handle = HandlePlayerInventoryMenu;
  PlayerInventoryData playerInventoryData = { .player = NULL };
  playerInventoryMenu->data = &playerInventoryData;

  pauseMenu = RegisterUIScreen("Pause Menu");
  pauseMenu->handle = HandlePauseMenu;
  PauseMenuData pauseMenuData = { .isExiting = NULL };
  pauseMenu->data = &pauseMenuData;

  debugMenu = RegisterUIScreen("Debug");
  debugMenu->handle = HandleDebugMenu;
  DebugMenuData debugMenuData = { .world = NULL, .player = NULL };
  debugMenu->data = &debugMenuData;
}
