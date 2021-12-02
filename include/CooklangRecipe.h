#ifndef INCLUDED_REC
#define INCLUDED_REC

#include "LinkedListLib.h"

#endif


// data structure definitions
// define recipe
typedef struct {

  // all datatypes in Recipe are required to be initialized, although not required to have any content
  List * metaData;

  List * stepList;

  List * equipmentList;

  List * ingredientList;

  List * shoppingList;

} Recipe;




// The metadata
typedef struct {

  // required
  char * identifier;
  // required
  char * content;

} Metadata;




typedef struct {
  // a NULL value means that there is none of the given argument in the direction
  // except for double, where -1 means none given
  
  // the type of direction, either textitem, ingredient, cookware, timer
  // required
  char * type;

  // the readable text value to be displayed on request - usually just the name
  // for text items it is just the text item string
  // required - except for timers were there can be no name
  char * value;

  // the # of unites required for the direction - not an option for cookware
  // -1 means that there is none defined in the recipe
  double quantity;

  // used instead of a quantity double
  // used when the input quantity is in the form of a string rather than a number
  char * quantityString;

  // the standard measurement for the quantity to follow, sometimes required for ingredient, not for cookware
  // only allowed if a quantity is present
  char * unit;

} Direction;



// A step
typedef struct {

  // just a list of directions
  List * directions;

} Step;




// shopping list
typedef struct {

  char * category;
  List * shoppingItem;

} ShoppingList;

typedef struct {
  
  // required, the first name
  char * name;

  // any other synonyms
  List * synonyms;

} ShoppingItem;









// * * * * * * * * * * * * * * * * * * * * 
// ******** Function Definitions *********
// * * * * * * * * * * * * * * * * * * * *
void test2();


Recipe * createRecipe();

void deleteRecipe( void * data );
char * recipeToString( void * data );


Metadata * createMetadata( char * identifier, char * content );

void deleteMetadata( void * data );
char * metadataToString( void * data );
int compareMetadata( const void * first, const void * second );


Direction * createDirection( char * type, char * value, char * quantityString, double quantity, char * unit );

void deleteDirection( void * data );
char * directionToString( void * data );
int compareDirections( const void * first, const void * second );


Step * createStep();

void deleteStep( void * data );
char * stepToString( void * data );
int compareSteps( const void * first, const void * second );