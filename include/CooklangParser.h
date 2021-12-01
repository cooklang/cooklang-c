#ifndef INCLUDED_PAR
#define INCLUDED_PAR

#include "LinkedListLib.h"
#include "CooklangRecipe.h"

#endif

// functions
void test();




typedef struct {

  // the origin file of the recipe
  char * fileName;

  // the recipe from the file
  Recipe * recipe;

  // pictures that go with the recipe steps
  // 
  List * pictures;

} CooklangParser;

