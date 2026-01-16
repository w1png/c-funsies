#include "ui.h"
#include "camera.h"
#include "crafting.h"
#include "objects.h"
#include "player.h"
#include "raylib.h"
#include <stdatomic.h>
#define RAYGUI_IMPLEMENTATION
#include "lib/raygui.h"
#include "const.h"

UIScreen ui_screens[MAX_UI_SCREENS];
int ui_screen_count = 0;

UIScreen* UI_LOCK = NULL;
bool CanSetUILock(int callerId) {
  return UI_LOCK == NULL || UI_LOCK->id == callerId;
}
int SetUILock(UIScreen* us, int callerId) {
  if (!CanSetUILock(callerId)) {
    return -1;
  }

  TraceLog(LOG_INFO, "Setting UI lock to %s", us != NULL ? us->name : "NULL");
  UI_LOCK = us;

  return 0;
}
UIScreen* GetUILock() {
  return UI_LOCK;
}

void SetScreenOpen(UIScreen* us, bool isOpen) {
  if (us->isBlocking) {
    if (SetUILock(isOpen ? us : NULL, us->id) != 0) {
      TraceLog(LOG_ERROR, "Failed to %s screen %s: Failed to set ui lock", isOpen ? "open" : "close", us->name);
      return;
    }
  }
  us->isOpen = isOpen;
} 


UIScreen* RegisterUIScreen(const char* name) {
  if (ui_screen_count >= MAX_UI_SCREENS) {
    TraceLog(LOG_ERROR, "Too many UI screens registered");
    exit(1);
  }

  TraceLog(LOG_INFO, "Registering UI screen: %s", name);
  UIScreen* us = &ui_screens[ui_screen_count++];
  us->id = ui_screen_count - 1;
  us->name = name;
  us->isOpen = false;
  us->isBlocking = false;

  TraceLog(LOG_INFO, "Registered UI screen: %s", name);
  return us;
} 

void HandleAllUIUpdates() {
  UIScreen* us;
  for (int i = 0; i < ui_screen_count; i++) {
    us = &ui_screens[i];
    if (us->isBlocking && !CanSetUILock(us->id)) {
      continue;
    };
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
    SetScreenOpen(us, false);
  }

  if (GuiButton((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*1 + GAP*1, width - PADDING*2, BUTTON_HEIGHT}, "Quit")) {
    SetScreenOpen(us, false);
    *data->isExiting = true;
  }
}

void HandlePlayerInventoryHUD(void* screen) {
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

  int totalButtons = 6;
  float width = 400.0f;
  float height = PADDING*2+BUTTON_HEIGHT*totalButtons+GAP*(totalButtons-1);
  Vector2 position = (Vector2){GetScreenWidth()/2.0f - width/2.0f, GetScreenHeight()/2.0f - height/2.0f};

  if (!us->isOpen) return;
  DrawBackground();
  DrawRectangle(position.x, position.y, width, height, WHITE);
  GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s FPS", data->features.showFPS ? "Hide" : "Show"), &data->features.showFPS);
  GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT + GAP, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s hovered tile type", data->features.showHoveredTileType ? "Hide" : "Show"), &data->features.showHoveredTileType);
  GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*2 + GAP*2, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s collisions", data->features.showCollisions ? "Hide" : "Show"), &data->features.showCollisions);
  GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*3 + GAP*3, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s break time", data->features.showBreakTime ? "Hide" : "Show"), &data->features.showBreakTime);
  GuiToggle((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*4 + GAP*4, width - PADDING*2, BUTTON_HEIGHT}, TextFormat("%s ui lock", data->features.showUILock ? "Hide" : "Show"), &data->features.showUILock);
  if (GuiButton((Rectangle){position.x + PADDING, position.y + PADDING + BUTTON_HEIGHT*(totalButtons-1) + GAP*(totalButtons-1), width - PADDING*2, BUTTON_HEIGHT}, "Close")) {
    SetScreenOpen(us, false);
  }
}

void HandleDebugInfo(void* screen) {
  UIScreen* us = screen;
  DebugMenuData* data = us->data;
  int fontSize = 16;

  if (data->features.showFPS) {
      const char *fpsText = 0;
      if (TARGET_FPS <= 0) fpsText = TextFormat("FPS: unlimited (%i)", GetFPS());
      else fpsText = TextFormat("FPS: %i (target: %i)", GetFPS(), TARGET_FPS);
      DrawText(fpsText, PADDING, GetScreenHeight()-PADDING-fontSize, fontSize, PINK);
  }

  if (data->features.showHoveredTileType) {
    if (*data->hoveredTile != NULL) {
      DrawText(TextFormat("Hovered tile type: %s", (*data->hoveredTile)->object->name), PADDING, GetScreenHeight()-PADDING-GAP*2-fontSize, fontSize, PINK);
    }
  }

  if (data->features.showUILock) {
    DrawText(TextFormat("UI Lock: %s", UI_LOCK != NULL ? UI_LOCK->name : "None"), PADDING, GetScreenHeight()-PADDING-GAP*3-fontSize, fontSize, PINK);
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
          camera
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
            camera
          ),
          6,
          1,
          WHITE
        );
      }
    }
  }
}

void HandleInventoryMenu(void* screen) {
  UIScreen* us = screen;
  InventoryMenuData* data = us->data;
  Player* player = data->player;

  float width = (PADDING+BUTTON_HEIGHT*MAX_INVENTORY_OBJECTS+GAP/2*MAX_INVENTORY_OBJECTS)*2+PADDING;
  float height = 400;
  Vector2 position = (Vector2){GetScreenWidth()/2.0f - width/2.0f, GetScreenHeight()/2.0f - height/2.0f};

  if (!us->isOpen) return;

  DrawBackground();
  DrawRectangle(position.x, position.y, width, height, WHITE);
  DrawRectangleLines(position.x, position.y, width/2, height, BLACK);
  DrawRectangleLines(position.x + width/2, position.y, width/2, height, BLACK);

  for (int i = 0; i < MAX_INVENTORY_OBJECTS; i++) {
    InventoryObject *object = player->inventory[i];
    int posX = position.x+PADDING+BUTTON_HEIGHT*i+GAP/2*i;
    int posY = position.y+PADDING;
    GuiButton((Rectangle){posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT}, "");
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

    // if (i == player->selectedInventoryObjectIndex) {
    //   DrawRectangle(posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT, (Color){0,255,0,100});
    // }
  }

  for (int i = 0; i < craftingRecipesCount; i++) {
    CraftingRecipe *recipe = &craftingRecipes[i];
    int posX = width/2+position.x+PADDING+BUTTON_HEIGHT*i+GAP/2*i;
    int posY = position.y+PADDING;
    int maxCraftAmount = GetMaxCraftAmount(recipe, player);
    if (GuiButton((Rectangle){posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT}, "")) {
      if (CraftRecipe(recipe, 1, player) != 0) {
        TraceLog(LOG_ERROR, "Failed to craft recipe %s", recipe->name);
        continue;
      }
    }
    DrawTexturePro(
      *recipe->result->texture,
      (Rectangle){ 0, 0, (float)recipe->result->texture->width, (float)recipe->result->texture->height },
      (Rectangle){ posX+2, posY+2, BUTTON_HEIGHT-4, BUTTON_HEIGHT-4},
      (Vector2){ 0, 0 },
      0.0f,
      WHITE
    );

    if (!maxCraftAmount) {
      DrawRectangle(posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT, (Color){255,0,0,100});
    }

    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT})) {
        int tooltipX = posX;
        int tooltipY = posY + BUTTON_HEIGHT + 6;

        int fontSize = 16;
        int lineHeight = fontSize + 2;

        int tooltipHeight = lineHeight*(recipe->ingredientsCount + 2)+GAP*2;
        int currentY = tooltipY + GAP;
        int tooltipWidth  = BUTTON_HEIGHT * 2.5;

        DrawRectangle(tooltipX, tooltipY, tooltipWidth, tooltipHeight + GAP*2,
                      WHITE);
        DrawRectangleLines(tooltipX, tooltipY, tooltipWidth, tooltipHeight + GAP*2,
                           maxCraftAmount > 0 ? GREEN : RED);


        DrawText(recipe->name, tooltipX + GAP, currentY, fontSize + 4, maxCraftAmount != 0 ? BLACK : RED);
        currentY += lineHeight + GAP;

        DrawText("Ingredients:", tooltipX + GAP, currentY, fontSize, BLACK);
        currentY += lineHeight;

        for (int j = 0; j < recipe->ingredientsCount; j++)
        {
            CraftingIngredient ingredient = recipe->ingredients[j];
            DrawText(TextFormat("  %s x%d", ingredient.object->name, ingredient.amount), tooltipX + GAP, currentY, fontSize, HasInventoryObject(player, ingredient.object, ingredient.amount) ? GREEN : RED);
            currentY += lineHeight;
        }
    }
  }
}

void HandleContainerMenu(void* screen) {
  UIScreen* us = screen;
  ContainerMenuData* data = us->data;
  Player* player = data->player;

  float width = (PADDING+BUTTON_HEIGHT*MAX_INVENTORY_OBJECTS+GAP/2*MAX_INVENTORY_OBJECTS)*2+PADDING;
  float height = 400;
  Vector2 position = (Vector2){GetScreenWidth()/2.0f - width/2.0f, GetScreenHeight()/2.0f - height/2.0f};

  if (!us->isOpen) return;

  DrawBackground();
  DrawRectangle(position.x, position.y, width, height, WHITE);
  DrawRectangleLines(position.x, position.y, width/2, height, BLACK);
  DrawRectangleLines(position.x + width/2, position.y, width/2, height, BLACK);

  ContainerObject* containerObjects = data->tile->data;
  for (int i = 0; i < MAX_INVENTORY_OBJECTS; i++) {
    InventoryObject *object = player->inventory[i];
    bool isFreeSlot = object == NULL || object->object == NULL || object->amount == 0;
    int posX = position.x+PADDING+BUTTON_HEIGHT*i+GAP/2*i;
    int posY = position.y+PADDING;
    if (GuiButton((Rectangle){posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT}, "")) {
      if (isFreeSlot) return;
      for (int j = 0; j < CHEST_SIZE; j++) {
        if (containerObjects[j].object == player->inventory[i]->object) {
          containerObjects[j].amount += player->inventory[i]->amount;
          player->inventory[i]->amount = 0;
          TraceLog(LOG_INFO, "Removing item from container %s", containerObjects[j].object->name);
          break;
        } else if (containerObjects[j].object == NULL || containerObjects[j].amount == 0) {
          containerObjects[j].amount = player->inventory[i]->amount;
          containerObjects[j].object = player->inventory[i]->object;
          player->inventory[i] = NULL;
          TraceLog(LOG_INFO, "Putting item into container %s", containerObjects[j].object->name);
          break;
        }
      }
    }
    if (!isFreeSlot) {
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
  }


  for (int i = 0; i < CHEST_SIZE; i++) {
    ContainerObject containerObject = containerObjects[i];
    bool isFreeSlot = containerObject.object == NULL || containerObject.amount == 0;

    int posX = width/2+position.x+PADDING+BUTTON_HEIGHT*i+GAP/2*i;
    int posY = position.y+PADDING;
    if (GuiButton((Rectangle){posX, posY, BUTTON_HEIGHT, BUTTON_HEIGHT}, "")) {
     if (isFreeSlot) return;

      Object* movedObject = containerObjects[i].object;
      int movedAmount = containerObjects[i].amount;

      for (int j = 0; j < MAX_INVENTORY_OBJECTS; j++) {
        if (player->inventory[j] && player->inventory[j]->object == movedObject) {
          player->inventory[j]->amount += movedAmount;
          containerObjects[i].object = NULL;
          containerObjects[i].amount = 0;

          TraceLog(LOG_INFO, "Stacked item into player inventory: %s", movedObject->name);
          return;
        }

        if (player->inventory[j] == NULL) {
          player->inventory[j] = calloc(1, sizeof(InventoryObject));
          player->inventory[j]->object = movedObject;
          player->inventory[j]->amount = movedAmount;

          containerObjects[i].object = NULL;
          containerObjects[i].amount = 0;

          TraceLog(LOG_INFO, "Inserted item into player inventory: %s", movedObject->name);
          return;
        }
      }
    }
    if (!isFreeSlot) {
      DrawTexturePro(
        *containerObject.object->texture,
        (Rectangle){ 0, 0, (float)containerObject.object->texture->width, (float)containerObject.object->texture->height },
        (Rectangle){ posX+2, posY+2, BUTTON_HEIGHT-4, BUTTON_HEIGHT-4},
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
      );
      DrawText(TextFormat("%i", containerObject.amount), posX+PADDING/4, posY, 24, WHITE);
    }
  }
}


void RegisterAllUIScreens() {
  inventoryHUD = RegisterUIScreen("Player Inventory");
  inventoryHUD->handle = HandlePlayerInventoryHUD;
  PlayerInventoryData playerInventoryData = { .player = NULL };
  inventoryHUD->data = &playerInventoryData;

  DebugMenuData debugMenuData = { .world = NULL, .player = NULL, .hoveredTile = NULL };
  debugHUD = RegisterUIScreen("Debug Info");
  debugHUD->handle = HandleDebugInfo;
  debugHUD->data = &debugMenuData;
  debugHUD->isOpen = true;

  debugMenu = RegisterUIScreen("Debug");
  debugMenu->handle = HandleDebugMenu;
  debugMenu->isBlocking = true;
  debugMenu->data = &debugMenuData;

  pauseMenu = RegisterUIScreen("Pause Menu");
  pauseMenu->handle = HandlePauseMenu;
  PauseMenuData pauseMenuData = { .isExiting = NULL };
  pauseMenu->data = &pauseMenuData;
  pauseMenu->isBlocking = true;

  inventoryMenu = RegisterUIScreen("Inventory");
  inventoryMenu->handle = HandleInventoryMenu;
  InventoryMenuData inventoryMenuData = { .player = NULL };
  inventoryMenu->data = &inventoryMenuData;
  inventoryMenu->isBlocking = true;

  ContainerMenuData containerMenuData = { .player = NULL, .tile = NULL };
  containerMenu = RegisterUIScreen("Container");
  containerMenu->data = &containerMenuData;
  containerMenu->isBlocking = true;
  containerMenu->handle = HandleContainerMenu;
}
