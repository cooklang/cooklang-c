#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/CooklangRecipe.h"


void test2(){
  printf("Test working\n");
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


char ** parseMetaString( char * metaString ){
  char ** results;
  char * token;
  
  // check input
  if( metaString == NULL ){
    return NULL;
  }

  // tokenize input
  token = strtok(metaString, ":");

  if( token == NULL ){
    return NULL;
  }

  // make space for two string pointers
  results = malloc(sizeof(char *) * 2);

  results[0] = malloc(sizeof(char) * strlen(token) + 1);
  strcpy(results[0], token);

  // tokenize again and check for a string
  token = strtok(NULL, ":");

  if( token == NULL ){
    results[1] = NULL;
    return results;
  }

  results[1] = malloc(sizeof(char) * strlen(token) + 1);
  strcpy(results[1], token);

  return results;

}


// * * * * * * * * * * * * * * * * * * * * 
// ******** Metadata Functions ***********
// * * * * * * * * * * * * * * * * * * * *



Metadata * createMetadata( char * metaString ){

  // parse the metadata first
  char ** results = parseMetaString(metaString);

  char * identifier = results[0];
  char * content = results[1];

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

  free(results[0]);
  free(results[1]);
  free(results);

  return tempMeta;
}

// parse meta data function



void deleteMetadata( void * data ){
  Metadata * meta = data;

  free(meta->content);
  free(meta->identifier);
  free(meta);
} 

char * metadataToString( void * data ){
  Metadata * meta = data;

  int length = strlen(meta->identifier) + strlen(meta->content) + 50;

  char * returnString = malloc(sizeof(char) * length);

  sprintf(returnString, "{ Identifier: \"%s\", Content: \"%s\" }", meta->identifier, meta->content);

  return returnString;
}

int compareMetadata( const void * first, const void * second ){
  return 0;
}








// * * * * * * * * * * * * * * * * * * * *
// ******** Direction Functions **********
// * * * * * * * * * * * * * * * * * * * *





Direction * createDirection( char * type, char * value, char * amountString ){
  char ** amountResults = NULL;
  char * quantityString;
  char * unit;
  double quantity;


  // check input - must be a type
  if( type == NULL ){
    return NULL;
  }

  // value can only be null if the type is a timer, this case comes from a no name timer
  if( strcmp(type, "Timer") != 0 && value == NULL ){
    return NULL;
  }

  // parse amountString
  if( amountString == NULL ){
    quantityString = NULL;
    quantity = -1;
    unit = NULL;

  } else {
    amountResults = parseAmountString(amountString);

    quantityString = amountResults[0];
    unit = amountResults[1];

    // check if quantity string can be a double
    quantity = strtod(quantityString, NULL);
  }
  

  // if quantity is a double, that means the quantity string can be represented as a double, so it should be saved as one
  // so convert it to a double if it can be else make it negative one and keep quantityString as is
  if( quantity > 0 ){
    quantityString = NULL;
  } else {
    quantity = -1;
  }

  Direction * tempDir = malloc(sizeof(Direction));

  if( tempDir == NULL ){
    printf("error, malloc failed - createDirection 1\n");
    return NULL;
  }

  tempDir->type = strdup(type);

  if( value != NULL ){
    tempDir->value = strdup(value);
  } else {
    tempDir->value = NULL;
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

  if( amountResults != NULL){
    free(amountResults[0]);
    free(amountResults[1]);
    free(amountResults);
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

  free(dir);
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
  Step * step = data;

  freeList(step->directions);
  free(step);
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

  sprintf(stepString, " Step: %s", dirString);

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
  
  // make a new recipe
  Recipe * tempRec = malloc(sizeof(Recipe));
  
  // initialize all the lists in the recipe

  // steps
  List * stepList = initializeList(stepToString, deleteStep, compareSteps);
  tempRec->stepList = stepList;

  // metadata
  List * metaDataList = initializeList(metadataToString, deleteMetadata, compareMetadata);
  tempRec->metaData = metaDataList;

  return tempRec;
}

void deleteRecipe( void * data ){
  // free all the lists

}

char * recipeToString( void * data ){
  return "empty recipe\n";
}


