#ifndef CRAFTING_H
#define CRAFTING_H

#include "objects.h"
#include "const.h"

typedef struct {
  Object *object;
  int amount;
} CraftingIngredient;

typedef struct {
  int id;
  const char *name;
  CraftingIngredient ingredients[MAX_INVENTORY_OBJECTS];
  int ingredientsCount;
  Object *result;
  int resultAmount;
} CraftingRecipe;

extern CraftingRecipe craftingRecipes[MAX_CRAFTING_RECIPES];
extern int craftingRecipesCount;

CraftingRecipe* FENCE_RECIPE;
CraftingRecipe* CHEST_RECIPE;

void RegisterAllCraftingRecipes();

#endif
