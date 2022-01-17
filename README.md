# [WIP] cook-in-c
Cooklang parser implemented in C


## Install:
Download the code as a zip file and extract, or use the git clone command to download:
```
git clone https://github.com/cooklang/cook-in-c.git
```

## Documentation


The **Recipe** struct:
```C
typedef struct {
  List * metaData;
  List * stepList;

} Recipe;
```
will hold all the metaData and steps in the file after parsing.

The **Recipe** structure uses linked lists to store the parsed data. Traversal through the data after parsing is handled by the linked list library. All of the functions and documentation for the linked list can be found and described thoroughly in the *LinkedListLib.h* file, under the *include* folder.

The **toString(List * list)** method can be used to convert a linked list to a string, using the **printData** method that is supplied on creation of the recipe (the default setup is used in the **createRecipe()** function below). The **printData** method for the **List * stepList** (a list of parsed steps from the input) will return a string in JSON format. The string includes a seperate JSON element for each direction that is found in the step, one direction per line.

To begin working with the parser, a Recipe has to be initialized with these steps:
```C
// setup the recipe
Recipe * finalRecipe = createRecipe();

// create the first step and insert it
Step * currentStep = createStep();
insertBack(finalRecipe->stepList, currentStep);
```

The yyparse function automatically pulls input from the variable **yyin**, which can be set to an input file, **STDIN**, or other forms of an input stream like this:
```C
yyin = file;
```
or:
```C
yyin  = stdin;
```
After creating the Recipe struct and setting the input stream, the yyparse() function can be used to parse the desired input, with the created recipe as input.

```C
// parse the file
yyparse(finalRecipe);
```
The final result will a be **Recipe** structure, as defined above, containing every direction in the input.


## Usage:
Cook-in-c can either be compiled as an executable to be run directly on linux systems, or it can be compiled as a shared library, which can then be used by other languages. There are more details about using the library in the _CONTRIBUTING.md_ file.


## Note:
There are many code points from the UTF-8 character database in the Cooklang.l file. This results in a very large state machine created by the flex library. As a result of this, compilation times of the executable and the shared library *can* be very long. Results may vary based on computer speeds.

