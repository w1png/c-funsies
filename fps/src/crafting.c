#include "crafting.h"
#include "objects.h"
#include "const.h"
#include <stdlib.h>

CraftingRecipe craftingRecipes[MAX_CRAFTING_RECIPES];
int craftingRecipesCount = 0;

CraftingRecipe *RegisterCraftingRecipe(const char* name) {
  if (craftingRecipesCount >= MAX_CRAFTING_RECIPES) {
    TraceLog(LOG_ERROR, "Too many crafting recipes registered");
    exit(1);
  }

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

  CHEST_RECIPE = RegisterCraftingRecipe("chest");
  CHEST_RECIPE->ingredients[0].object = WOOD;
  CHEST_RECIPE->ingredients[0].amount = 8;
  CHEST_RECIPE->ingredientsCount = 1;
  CHEST_RECIPE->resultAmount = 1;
  CHEST_RECIPE->result = CHEST;

  TraceLog(LOG_INFO, "Crafting recipes registered");
}
