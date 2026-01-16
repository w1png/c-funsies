#include "camera.h"
#include <math.h>
#include "const.h"
#include "player.h"

Camera2D camera = { 0 };

void InitCamera() {
  camera.zoom = 0.5f;
}

void HandleCameraZoom() {
  camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
  if (MIN_ZOOM < camera.zoom)
    camera.zoom = MIN_ZOOM;
  else if (camera.zoom < MAX_ZOOM) camera.zoom = MAX_ZOOM;
}

void HandleCameraTarget(Player* player) {
  camera.target = (Vector2){ player->bounds.x + 20.0f, player->bounds.y + 20.0f };
  camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
}
