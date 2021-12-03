#ifndef INCLUDED_PAR
#define INCLUDED_PAR

#include "LinkedListLib.h"
#include "CooklangRecipe.h"

#endif




typedef struct {

  // the origin file of the recipe
  char * fileName;

  // the recipe from the file
  Recipe * recipe;

  // pictures that go with the recipe steps
  // 
  List * pictures;

} CooklangParser;



// functions
void test();


char * addTwoStrings(char * first, char * second);
char * addThreeStrings(char * first, char * second, char * third);

void addDirection( Recipe * recipe, char * type, char * value, char * amountString );

void addMetaData( Recipe * recipe, char * metaDataString );
