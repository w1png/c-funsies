#include "crafting.h"
#include "objects.h"
#include "const.h"

CraftingRecipe craftingRecipes[MAX_CRAFTING_RECIPES];
int craftingRecipesCount = 0;

CraftingRecipe *RegisterCraftingRecipe(const char* name) {
  TraceLog(LOG_INFO, "Registering crafting recipe %s", name);
  CraftingRecipe *recipe = &craftingRecipes[craftingRecipesCount++];
  recipe->id = craftingRecipesCount;
  recipe->name = name;
  TraceLog(LOG_INFO, "Crafting recipe %s registered", name);
  return recipe;
}

void RegisterAllCraftingRecipes() {
  TraceLog(LOG_INFO, "Registering crafting recipes");
  FENCE_RECIPE = RegisterCraftingRecipe("fence");
  FENCE_RECIPE->ingredients[0].object = WOOD;
  FENCE_RECIPE->ingredients[0].amount = 5;
  FENCE_RECIPE->ingredientsCount = 1;
  FENCE_RECIPE->resultAmount = 3;
  FENCE_RECIPE->result = FENCE;

  TraceLog(LOG_INFO, "Crafting recipes registered");
}
