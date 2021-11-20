#ifndef INCLUDED_PAR
#define INCLUDED_PAR

#include "LinkedListLib.h"

#endif

// functions
int test();


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


// A quantity, such as weight or volume
typedef struct {

  // both required
  // 
  char * unit;
  float amount;

} Quantity;



// An ingredient
typedef struct {

  // one word or multiword
  char * title;
  // not required, defualt 1
  float amount;
  // a quantity, not required, default null if there is none, or if the amount is used
  Quantity * quantity;

} Ingredient;



// The metadata
typedef struct {

  // dunno which are required
  char * source;
  char * meal;
  char * prepTime;
  char * servings;
  char * scaling;

} Metadata;


// A cookware
typedef struct {

  // required - single word or multiword
  char * title;

} Cookware;

typedef struct {
  // the type of direction, either textitem, ingredient, cookware, timer
  char * type;
  // the readable text value to be displayed on request
  char * value;
  // the title of the item required for the direction - not required for timers, not required for text item
  char * name;
  // the standard measurement for the quantity to follow, sometimes required for ingredient, not for cookware
  char * unit;
  // the amount of the item required for the direction - not an option for cookware
  double quantity;

} Direction;


// A step
typedef struct {

  // required - make JSON?
  // otherwise list of directions
  char * instruction;
  // not required
  char * picture;

} Step;

// A timer
typedef struct {

  // not required
  char * name;

} Timer;

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



// parser stuff
typedef struct {

  // the origin file of the recipe
  char * fileName;

  // the recipe from the file
  Recipe * parsedRecipe;

} CooklangParser;