#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define RUN_MULTIPLIER 2.0f
#define TARGET_FPS 120
#define POINT_SIZE 48
#define PLAYER_SPEED (POINT_SIZE * 3)
#define MIN_ZOOM ((float)POINT_SIZE / 12)
#define MAX_ZOOM 0.5f
#define MAX_WORLD_SIZE 100
#define TIME_LIMIT_PER_BOMB .7f
#define PLAYER_SCALE 0.8f
#define MAX_INVENTORY_OBJECTS 7
#define CHEST_SIZE 7
#define INTERACT_DISTANCE 5.0f

#define MAX_OBJECTS 32
#define MAX_TEXTURES 32
#define MAX_UI_SCREENS 16
#define MAX_CRAFTING_RECIPES 16
#define MAX_PARTICLE_EMITTERS 16

#define BUTTON_HEIGHT 48
#define PADDING 12
#define GAP 8

#define HAS_TAG(obj, tag)       ((obj)->tags & (tag))
#define HAS_ALL_TAGS(obj, mask) (((obj)->tags & (mask)) == (mask))
