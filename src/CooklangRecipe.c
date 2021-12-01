#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/CooklangRecipe.h"


void test2(){
  printf("Test working\n");
}







// * * * * * * * * * * * * * * * * * * * * 
// ******** Metadata Functions ***********
// * * * * * * * * * * * * * * * * * * * *




void deleteMetadata( void * data ){

}

char * metadataToString( void * data ){
  return "empty metadata \n";
}

int compareMetadata( const void * first, const void * second ){
  return 0;
}


Metadata * createMetadata( char * identifier, char * content ){

  // check valid input
  if( identifier == NULL || content == NULL ){
    return NULL;
  }

  Metadata * tempMeta = malloc(sizeof(Metadata));

  if( tempMeta == NULL ){
    printf("error, malloc failed - createMetadata1\n");
    return NULL;
  }

  // create space for and copy in the input strings
  tempMeta->identifier = strdup(identifier);
  tempMeta->content = strdup(content);

  return tempMeta;
}





// * * * * * * * * * * * * * * * * * * * * 
// ******** Direction Functions **********
// * * * * * * * * * * * * * * * * * * * *





Direction * createDirection( char * type, char * value, char * quantityString, double quantity, char * unit ){

  // check input - must be a type
  if( type == NULL ){
    return NULL;
  }

  // value can only be null if the type is a timer, this case comes from a no name timer
  if( strcmp(type, "timer") != 0 && value == NULL ){
    return NULL;
  }

  Direction * tempDir = malloc(sizeof(Direction));

  if( tempDir == NULL ){
    printf("error, malloc failed - createDirection 1\n");
    return NULL;
  }

  tempDir->type = strdup(type);

  if( value != NULL ){
    tempDir->value = strdup(value);
  }

  // there is no quantity and the direction is done
  if( quantity == -1 && quantityString == NULL ){
    tempDir->quantity = -1;
    tempDir->quantityString = NULL;
    tempDir->unit = NULL;

    return tempDir;
  }

  if( quantity == -1 ){
    // quantity must be a string
    tempDir->quantityString = strdup(quantityString);
    tempDir->quantity = -1;
  } else {
    // must be a double
    tempDir->quantity = quantity;
    tempDir->quantityString = NULL;
  }

  // if unit input, set, else, set null
  if( unit != NULL ){
    tempDir->unit = strdup(unit);
  } else {
    tempDir->unit = NULL;
  }

  return tempDir;
}

void deleteDirection( void * data ){
  Direction * dir = data;

  // free all the strings
  free(dir->type);
  free(dir->value);
  free(dir->quantityString);
  free(dir->unit);
}

char * directionToString( void * data ){
  Direction * dir = data;

  char * tempString = NULL;

  int length = 0;

  if( dir->type != NULL ){
    length += strlen(dir->type);
  }

  if( dir->value != NULL ){
    length += strlen(dir->value);
  }

  if( dir->quantityString != NULL ){
    length += strlen(dir->quantityString);
  }
  
  if( dir->unit != NULL ){
    length += strlen(dir->unit);
  }

  length += 70;

  if( length != 15 ){
    tempString = malloc( sizeof(char) * length );
  } else {
    return NULL;
  }

  if( tempString == NULL ){
    printf("error, malloc failed - directionToString1\n");
    return NULL;
  }

  // make an output string based on what kind it is

  // if its a text iten use value instead of name
  if( strcmp(dir->type, "TextItem") == 0 ){
    sprintf(tempString, "{ type: \"%s\", value: \"%s\" }", dir->type, dir->value);

  // timers, cookware, and ingredients
  } else {
    // name and value
    if( dir->value != NULL ){
      sprintf(tempString, "{ type: \"%s\", name: \"%s\"", dir->type, dir->value);
    } else {
      sprintf(tempString, "{ type: \"%s\"", dir->type);
    }

    // quantity, string format
    if( dir->quantityString != NULL ){
      char * tempQuanString = malloc(sizeof(char) * strlen(dir->quantityString) + 20);
      sprintf(tempQuanString, ", quantity: \"%s\"", dir->quantityString);

      strcat(tempString, tempQuanString);
      free(tempQuanString);
    // quantity, double format
    } else if( dir->quantity != -1 ){
      char * tempQuanString = malloc(sizeof(char) * 20);
      sprintf(tempQuanString, ", quantity: %.3f", dir->quantity);

      strcat(tempString, tempQuanString);
      free(tempQuanString);
    // no quantity - end of string
    } else {
      strcat(tempString, " }");
      return tempString;
    }

    // add the unit if there is one, else close the tag
    // only ingredients/timers
    if( strcmp(dir->type, "Cookware") != 0 ){
      if( dir->unit != NULL ){
        char * unitString = malloc(sizeof(dir->unit) + 20);
        sprintf(unitString, ", units: \"%s\"", dir->unit);

        strcat(tempString, unitString);
        free(unitString);
      }
    }
    
    strcat(tempString, " }");
  }

  return tempString;
}

int compareDirections( const void * first, const void * second ){
  return 0;
}





// * * * * * * * * * * * * * * * * * * * * 
// *********  Step Functions  ************
// * * * * * * * * * * * * * * * * * * * *


Step * createStep(){

  // initalize a list of directions
  Step * tempStep = malloc(sizeof(Step));

  List * dirList = initializeList( directionToString, deleteDirection, compareDirections );

  tempStep->directions = dirList;

  return tempStep;
}

void deleteStep( void * data ){

}

char * stepToString( void * data ){
  Step * step = data;
  // get the direcitions string
  
  char * dirString;

  if( getLength(step->directions) != 0 ){
    dirString = toString(step->directions);
  } else {
    dirString = malloc(sizeof(char) * 50);
    sprintf(dirString, "Empty step\n");
  }

  char * stepString = malloc(sizeof(char) * strlen(dirString) + 20);

  sprintf(stepString, " Step:%s", dirString);

  free(dirString);

  return stepString;
}

int compareSteps( const void * first, const void * second){
  return 0;
}





// * * * * * * * * * * * * * * * * * * * * 
// ********  Recipe Functions  ***********
// * * * * * * * * * * * * * * * * * * * *

// creation functions

Recipe * createRecipe(){
  // initialize all lists in the recipe
  Recipe * tempRec = malloc(sizeof(Recipe));
  // steps
  List * stepList = initializeList( stepToString, deleteStep, compareSteps );

  tempRec->stepList = stepList;

  return tempRec;
}

void deleteRecipe( void * data ){
  // free all the lists

}

char * recipeToString( void * data ){
  return "empty recipe\n";
}


int compareRecipes( const void * first, const void * second ){
  return 0;
}

