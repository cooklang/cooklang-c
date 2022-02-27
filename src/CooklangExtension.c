#include <Python.h>

#include "../include/CooklangParser.h"

// python wrapper methods


// parse a recipe 
static PyObject * methodParseRecipe(PyObject *self, PyObject * args){
  int check;
  int dirLength;
  char * fileName = NULL;

  ListIterator stepIter;
  Step * curStep;
  ListIterator dirIter;
  Direction * curDir;
  ListIterator metaIter;
  Metadata * curMeta;

  PyObject * metaListObject;
  PyObject * metaObject;
  
  PyObject * stepListObject;
  PyObject * stepObject;
  PyObject * directionObject;

  PyObject * ingredientListObject;
  PyObject * cookwareListObject;
  

  // get args - no embedded null code points
  if(!PyArg_ParseTuple(args, "s", &fileName)){
    return NULL;
  }

  // parse the recipe recipe
  Recipe * parsedRecipe = parseRecipe(fileName);
  
  // build a python object to represent the recipe
  PyObject * recipeObject = PyDict_New();





  // add all the metadata

  // loop through each metadata
  metaIter = createIterator(parsedRecipe->metaData);
  curMeta = nextElement(&metaIter);

  metaListObject = PyList_New(0);

  while( curMeta != NULL ){
    // create the meta data python object
    metaObject = Py_BuildValue("{s:s}", curMeta->identifier, curMeta->content);

    // add it to the list
    check = PyList_Append(metaListObject, metaObject);
    if( check == -1 ){
      printf("Error adding new meta data to list\n");
    }

    // get the next one
    curMeta = nextElement(&metaIter);
  }

  // add the whole list to the final object
  check = PyDict_SetItemString(recipeObject, "metadata", metaListObject);
  if( check == -1 ){
    printf("Error adding meta data list to recipe object\n");
  }



  // the lists to store steps/ingredients/cookware
  stepListObject = PyList_New(0);
  ingredientListObject = PyList_New(0);
  cookwareListObject = PyList_New(0);

  // add all the steps
  
  // loop through every step
  stepIter = createIterator(parsedRecipe->stepList);
  curStep = nextElement(&stepIter);

  while( curStep != NULL ){
    // only add non-empty steps
    dirLength = getLength(curStep->directions);

    if(dirLength > 0 ){

      // loop through every direction
      dirIter = createIterator(curStep->directions);
      curDir = nextElement(&dirIter);

      // create a new list of directions
      stepObject = PyList_New(0);

      while( curDir != NULL ){
        // build a pyobject out of its values

        // if its a text direction
        if( strcmp(curDir->type, "text") == 0 ){
          directionObject = Py_BuildValue("{s:s, s:s}", "type", "text", "value", curDir->value);
        
        // other kinds
        } else {
          directionObject = PyDict_New();
          // add each attribute of the direction to the pyobject

          // type
          check = PyDict_SetItemString(directionObject, "type", PyUnicode_FromString(curDir->type));
          if( check == - 1 ){
            printf("Error adding type to new direction object\n");
          }

          // value
          if( curDir->value != NULL ){
            check = PyDict_SetItemString(directionObject, "name", PyUnicode_FromString(curDir->value));
            if( check == - 1 ){
              printf("Error adding name to new direction object\n");
            } 
          } else {
            check = PyDict_SetItemString(directionObject, "name", PyUnicode_FromString(""));
            if( check == - 1 ){
              printf("Error adding name to new direction object\n");
            }
          }

          // quantity 
          if( curDir->quantityString == NULL ){
            if( curDir->quantity != -1 ){
              check = PyDict_SetItemString(directionObject, "quantity", PyFloat_FromDouble(curDir->quantity));
              if( check == - 1 ){
                printf("Error adding quantity to new direction object\n");
              }
            }
          } else {
            check = PyDict_SetItemString(directionObject, "quantity", PyUnicode_FromString(curDir->quantityString));
            if( check == - 1 ){
              printf("Error adding quantity to new direction object\n");
            }
          }

          // unit
          if(strcmp(curDir->type, "cookware") != 0 ){
            if( curDir->unit != NULL ){
              check = PyDict_SetItemString(directionObject, "units", PyUnicode_FromString(curDir->unit));
              if( check == - 1 ){
                printf("Error adding units to new direction object\n");
              }
            } else {
              check = PyDict_SetItemString(directionObject, "units", PyUnicode_FromString(""));
              if( check == - 1 ){
                printf("Error adding units to new direction object\n");
              }
            }
          }
        }

        // add it to the step
        check = PyList_Append(stepObject, directionObject);
        if(check == -1){
          printf("Error adding new direction to direction list\n");
        }

        // if its an ingredient add it to the ingredient list
        if( strcmp(curDir->type, "ingredient") == 0 ){
         check = PyList_Append(ingredientListObject, directionObject);

          if(check == -1){
            printf("Error adding new direction to direction list\n");
          } 
        }

        // if its a cookware add it to the cookware list
        if( strcmp(curDir->type, "cookware") == 0 ){
         check = PyList_Append(cookwareListObject, directionObject);

          if(check == -1){
            printf("Error adding new direction to direction list\n");
          } 
        }

        // get the next direction
        curDir = nextElement(&dirIter);
      }
      check = PyList_Append(stepListObject, stepObject);
      if(check == -1){
        printf("Error adding step to step list\n");
      }
    }

    // get the next step
    curStep = nextElement(&stepIter);
  }

  // add ingredient list to the recipe
  check = PyDict_SetItemString(recipeObject, "ingredients", ingredientListObject);
  if(check == -1){
    printf("Error adding step list to recipe\n");
  }

  // add cookware list to the recipe
  check = PyDict_SetItemString(recipeObject, "cookware", cookwareListObject);
  if(check == -1){
    printf("Error adding step list to recipe\n");
  }

  //add steps to the recipe
  check = PyDict_SetItemString(recipeObject, "steps", stepListObject);
  if(check == -1){
    printf("Error adding step list to recipe\n");
  }

  // return
  return (PyObject *) recipeObject;
}








// python module methods array
static PyMethodDef cooklangCMethods[] = {
  {"parseRecipe", methodParseRecipe, METH_VARARGS, "Python wrapper function for the parseRecipe() function written in C."},
  // {"printRecipe", methodPrintRecipe, METH_VARARGS, "Python wrapper function for printing the contents of a recipe in C."},
  {NULL, NULL, 0, NULL}
};

// define the module
static PyModuleDef cooklangC = {
    PyModuleDef_HEAD_INIT,
    .m_name = "cooklangC",
    .m_doc = "A python module that uses C to parse recipe files using cooklang.",
    .m_size = -1,
    cooklangCMethods
};

// initialization function 
PyMODINIT_FUNC PyInit_cooklangC(void){
  return PyModule_Create(&cooklangC);
}