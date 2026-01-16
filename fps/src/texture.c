#include "texture.h"
#include "raylib.h"
#include <stdlib.h>

Textures textures;

Texture2D LoadTextureWithFallback(const char* path) {
  TraceLog(LOG_INFO, "Loading texture %s", path);
  Texture2D texture = LoadTexture(path);
  if (!IsTextureValid(texture)) {
    TraceLog(LOG_ERROR, "Failed to load texture %s", path);
    texture = textures.placeholder;
  } else {
    TraceLog(LOG_INFO, "Texture %s loaded", path);
  }

  return texture;
}

void LoadTextures() {
  TraceLog(LOG_INFO, "Loading textures");
  textures.placeholder = LoadTextureWithFallback("./assets/textures/placeholder.png");
  if (!IsTextureValid(textures.placeholder)) {
    TraceLog(LOG_ERROR, "Failed to load texture ./assets/textures/placeholder.png");
    exit(1);
  }

  textures.player = LoadTextureWithFallback("./assets/textures/player.png");
  textures.wall = LoadTextureWithFallback("./assets/textures/wall.png");
  textures.tree = LoadTextureWithFallback("./assets/textures/pine_tree.png");
  textures.water = LoadTextureWithFallback("./assets/textures/water.png");
  textures.grass = LoadTextureWithFallback("./assets/textures/grass.png");
  textures.stone = LoadTextureWithFallback("./assets/textures/stone.png");
  textures.wood = LoadTextureWithFallback("./assets/textures/wood.png");
  textures.chest = LoadTextureWithFallback("./assets/textures/chest.png");
  TraceLog(LOG_INFO, "Textures loaded");
}
