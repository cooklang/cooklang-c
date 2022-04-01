
#include <stdlib.h>
#include <stdio.h>

#include "../include/CooklangParser.h"
#include "../parserFiles/Cooklang.tab.h"


extern FILE * yyin;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(char * str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);



// wrapper functions
// this function will parse the recipe from a string
Recipe * parseRecipeString( char * inputRecipeString ){

  // setup the recipe
  Recipe * finalRecipe = createRecipe();

  // create the first step
  Step * currentStep = createStep();
  insertBack(finalRecipe->stepList, currentStep);

  // add a newline at the end of the string to prevent errors in the parser
  char * newInputString = malloc(sizeof(char) * (strlen(inputRecipeString) + 20));
  sprintf(newInputString, "%s\n", inputRecipeString);

  // feed input to lexer
  YY_BUFFER_STATE buffer = yy_scan_string(newInputString);
  
  // parser
  yyparse(finalRecipe);
  yy_delete_buffer(buffer);

  return finalRecipe;
}


// this function parses a string that is made up of seperate lines
Recipe * parseMultipleRecipeStrings( char * inputRecipeString ){
  char * token;
  char * savePtr;
  Recipe * newRecipe;
  Recipe * totalRecipe;

  totalRecipe = createRecipe();

  token = strtok_r(inputRecipeString, "\n", &savePtr);

  // split each line up and parse each one
  while( token != NULL ){
    // get recipe for current string
    newRecipe = parseRecipeString(token);

    // add new recipe to total recipe
    combineRecipes(newRecipe, &totalRecipe);

    token = strtok_r(savePtr, "\n", &savePtr);
  }

  return totalRecipe;
}


// adds the new recipe to the original recipe
void combineRecipes( Recipe * newRecipe, Recipe ** originalRecipe ){
  Metadata * newMeta;
  Step * newStep;
  ListIterator metaIter;
  ListIterator stepIter;

  // add new metadata
  metaIter = createIterator(newRecipe->metaData);
  while( newMeta = nextElement(&metaIter), newMeta != NULL ){
    insertBack((*originalRecipe)->metaData, newMeta);
  }

  // add new steps
  stepIter = createIterator(newRecipe->stepList);
  while( newStep = nextElement(&stepIter), newStep != NULL ){
    if(getLength(newStep->directions) != 0){
      insertBack((*originalRecipe)->stepList, newStep);
    }
  }
}



Recipe * parseRecipe( char * fileName ){

  FILE * file;

  if( fileName != NULL ){
    file = fopen(fileName, "r");
    if( file != NULL ){
      yyin = file;
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }


  // setup the recipe
  Recipe * finalRecipe = createRecipe();

  // create the first step
  Step * currentStep = createStep();
  insertBack(finalRecipe->stepList, currentStep);

  yyparse(finalRecipe);

  fclose(file);

  return finalRecipe;
}



// function to get the i-th step in a recipe

// function to get the i-th direction in a step

// function to return the number of steps in a recipe

// function to return the number of direction in a step

// function to nicely print the step data

// function to nicely print the direction data






char * addTwoStrings(char * first, char * second){
  char * result = NULL;
  int length = 0;

  if( first == NULL || second == NULL){
    return NULL;
  }

  length += strlen(first);
  length += strlen(second);
  length += 5;

  result = malloc(sizeof(char) * length);

  if( result == NULL ){
    printf("Error, malloc failed");
    return NULL;
  }

  sprintf(result, "%s%s", first, second);


  return result;
}


char * addThreeStrings(char * first, char * second, char * third){
  char * result = NULL;
  int length = 0;

  if( first == NULL || second == NULL || third == NULL ){
    return NULL;
  }

  length += strlen(first);
  length += strlen(second);
  length += strlen(third);
  length += 5;

  result = malloc(sizeof(char) * length);

  if( result == NULL ){
    printf("Error, malloc failed");
    return NULL;
  }

  sprintf(result, "%s%s%s", first, second, third);


  return result;
}


// adds the new direction corresponding to arguments 2-6, to the recipe in argument 1
// always adds to the last step in the list
void addDirection( Recipe * recipe, char * type, char * value, char * amountString ){

  // create a direction then add it to the direction list
  Direction * tempDir = createDirection(type, value, amountString);

  Step * step = getFromBack(recipe->stepList);

  if( tempDir != NULL ){
    insertBack(step->directions, tempDir);
  }

  // if cookware/ingredient, add to the appropriate lists
  if( strcmp(type, "Cookware") == 0 ){
    insertBack(step->equipmentList, tempDir);
  }

  if( strcmp(type, "Ingredient") == 0 ){
    insertBack(step->ingredientList, tempDir);
  }
}


void addMetaData( Recipe * recipe, char * metaDataString ){

  // create a new metadata and add it to the list at the back
  Metadata * tempMeta = createMetadata(metaDataString);

  if( tempMeta != NULL ){
    insertBack(recipe->metaData, tempMeta);
  }
}
