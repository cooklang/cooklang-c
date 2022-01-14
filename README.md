# [WIP] cook-in-c
Cooklang parser implemented in C


## Install:
Download the code as a zip file and extract, or use the git clone command to download:
```
git clone https://github.com/cooklang/cook-in-c.git
```

## Usage:
Cook-in-c can either be compiled as an executable to be run directly on linux systems, or it can be compiled as a shared library, which can then be used by other languages.

### Documentation:
The recipe struct: 
```C
typedef struct {
  List * metaData;
  List * stepList;

} Recipe;

```
will hold all the metaData and steps in the file after parsing with the yyparse() function, with the previously allocated recipe. A recipe can be made for use via:
```C
// setup the recipe
  Recipe * finalRecipe = createRecipe();

  // create the first step and insert it
  Step * currentStep = createStep();
  insertBack(finalRecipe->stepList, currentStep);

  // parse the file
  yyparse(finalRecipe);
```
The yyparse file automatically pulls input from the variable 'yyin', which can be set to an input file, STDIN, or other forms of an input stream like this:
```C
  yyin = file;
```
or:
```C
  yyin  = stdin;
```


### via C
To compile the code as an executable use the commands:
```
make clean
make parser
```
This will compile the executable into a file with the default name './a.out'

While using the executable file, the desired file can be chosen to parse by writing as the first argument:
```
./a.out 'fileName.cook'
```

### via Python/other
To compile the code as an executable use the commands:
```
make clean
make parser_library
```
While using the shared library:
-  the function 'testFile' can be used to parse a file, and returns a string which contains a JSON element for each direction in the file, one JSON element per line.
- the function 'runFile' can be used in the same way, with the same output, but the function prints the output to the command line.
- 

## Note:
There are many code points from the UTF-8 character database in the Cooklang.l file. This results in a very large state machine created by the flex library. As a result of this, compilation times of the executable and the shared library *can* be very long. Results may vary based on computer speeds.

## Contribute:
If you are using the library, and find any problems, or even have any suggestions, please open an issue on the github and we can work towards a solution.