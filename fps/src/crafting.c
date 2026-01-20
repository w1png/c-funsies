#include "crafting.h"
#include "objects.h"
#include "const.h"
#include <stdlib.h>

CraftingRecipes craftingRecipes;
CraftingRecipe craftingRecipesArr[MAX_CRAFTING_RECIPES];
int craftingRecipesCount = 0;

CraftingRecipe *RegisterCraftingRecipe(const char* name) {
  if (craftingRecipesCount >= MAX_CRAFTING_RECIPES) {
    TraceLog(LOG_ERROR, "Too many crafting recipes registered");
    exit(1);
  }

  TraceLog(LOG_INFO, "Registering crafting recipe %s", name);
  CraftingRecipe *recipe = &craftingRecipesArr[craftingRecipesCount++];
  recipe->id = craftingRecipesCount;
  recipe->name = name;
  TraceLog(LOG_INFO, "Crafting recipe %s registered", name);
  return recipe;
}

void RegisterAllCraftingRecipes() {
  TraceLog(LOG_INFO, "===Registering crafting recipes===");

  craftingRecipes.fence = RegisterCraftingRecipe("fence");
  craftingRecipes.fence->ingredients[0].object = objects.wood;
  craftingRecipes.fence->ingredients[0].amount = 5;
  craftingRecipes.fence->ingredientsCount = 1;
  craftingRecipes.fence->resultAmount = 3;
  craftingRecipes.fence->result = objects.fence;

  craftingRecipes.chest = RegisterCraftingRecipe("chest");
  craftingRecipes.chest->ingredients[0].object = objects.wood;
  craftingRecipes.chest->ingredients[0].amount = 8;
  craftingRecipes.chest->ingredientsCount = 1;
  craftingRecipes.chest->resultAmount = 1;
  craftingRecipes.chest->result = objects.chest;

  TraceLog(LOG_INFO, "===Crafting recipes registered===");
}
