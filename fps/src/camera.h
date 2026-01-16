#ifndef CAMERA_H
#define CAMERA_H

#include "player.h"
#include "raylib.h"

extern Camera2D camera;

void InitCamera();
void HandleCameraZoom();
void HandleCameraTarget(Player* player);

#endif
