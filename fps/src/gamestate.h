#include <stdint.h>

typedef enum {
  TAG_PLAYING   = 1 << 0,
} GameStateTag;

typedef struct {
  uint32_t tags;
} GameState;
