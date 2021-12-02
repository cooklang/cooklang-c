
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

  sprintf(result, "%s %s", first, second);


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

  sprintf(result, "%s %s %s", first, second, third);

 

  return result;
}


// adds the new direction corresponding to arguments 2-6, to the recipe in argument 1
// always adds to the last step in the list
void addDirection(Recipe * recipe, char * type, char * value, char * quantityString, double quantity, char * unit){

  // create a direction then add it to the direction list
  Direction * tempDir = createDirection(type, value, quantityString, quantity, unit);

  Step * step = getFromBack(recipe->stepList);

  insertBack(step->directions, tempDir);
}




// amount can be:
//  - just a number - number + unit - just a word - word + unit - just a multiword - multiword + unit

char ** parseAmountString( char * amountString ){

  if( amountString == NULL ){
    return NULL;
  }

  char ** results = malloc(sizeof(char *) * 2);
  char * quantityDest = NULL;
  char * unitDest = NULL;
  
  // check if empty - an empty amount - if so set null results and return
  if( amountString[0] == '\0' ){
    results[0] = NULL;
    results[1] = NULL;
    return results;
  }

  // all other cases either string or string + unit string
  char * token = strtok(amountString, "%");

  if( token != NULL ){
    quantityDest = malloc(sizeof(char) * strlen(token) + 1);
    strcpy(quantityDest, token); 
  } else {
    // no quantity therefore no unit, set null results and return
    results[0] = NULL;
    results[1] = NULL;
    return results;
  }

  token = strtok(NULL, "%");

  if( token != NULL ){
    unitDest = malloc(sizeof(char) * strlen(token) + 1);
    strcpy(unitDest, token);
  } else {
    unitDest = NULL;
  }

  results[0] = quantityDest;
  results[1] = unitDest;


  return results;
}
