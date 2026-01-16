#ifndef TEXTURE_H
#define TEXTURE_H

#include "raylib.h"

typedef struct {
  Texture2D placeholder;
  Texture2D player;
  Texture2D wall;
  Texture2D tree;
  Texture2D water;
  Texture2D grass;
  Texture2D stone;
  Texture2D wood;
  Texture2D chest;
} Textures;

extern Textures textures;

void LoadTextures(void);

#endif
