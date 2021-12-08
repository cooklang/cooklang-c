
#include <stdlib.h>
#include <stdio.h>

#include "CooklangParser.h"


void test(){
  printf("test\n");
}


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

