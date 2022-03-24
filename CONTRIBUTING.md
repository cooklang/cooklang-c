


# Contributing
If you are using the library, and find any problems, or even have any suggestions, please open an issue on the github and we can work towards a solution.

## How to contribute:

### Install:

Download the code as a zip file and extract, or use the git clone command to download:

```
git clone https://github.com/cooklang/cook-in-c.git

```
If you would like to work with the library you will need the following packages installed: make and gcc for the compilation. These can all be installed via:
```
sudo apt-get install make gcc
```

The library is built using these versions:
```
GNU Make 4.2.1
gcc (Debian 8.3.0-6) 8.3.0
```
The library was built on: Debian GNU/Linux 10




### Run as an executable:

To compile the code as an executable use the command:
```
make parser
```

This will compile the executable into a file with the default name './a.out'.
While using the executable file, the desired file can be chosen to be parsed by writing as the first argument(without the quotations):

```
./a.out 'fileName.cook'
```
The output will be printed to stdout.


### Run as a shared library:
To use the library via a higher level language, the language must be installed. In this library, python was used to do the testing, with version 2.7.16. A library like *ctypes* for python can be used to interact with the shared library file. The user must also be able to run C executables.


To compile the code as a shared library use the command:
```
make library
```


While using the shared library:
-  the function 'testFile' can be used to parse a file, and returns a string which contains a JSON element for each direction in the file, one JSON element per line.
- the function 'runFile' can be used in the same way, with the same output, but the function prints the output to the command line.



## Testing:
To test the parser against some of your own test cases or files, the _testing.py_ file under the _testing_ folder can be used. It is written in Python and uses **unittest** to perform the tests.

1. The script takes input tests in YAML format, and automatically extracts them from the _testing/tests.yaml_ file.

2. To run the script simply type the following into a command line in the same directory as the _testing.py_ file:
```
python -m unittest testing.testing
```

The output will be printed to the command line, where each test will look something like this (depending on the test):
```
____  Test: testSlashInText
____           Expected Output           ____
  Step 0:
   Item 1:
    Type: text
    Value: Fry for
   Item 2:
    Type: timer
    Name: potato

  Metadata:
____            Actual Output            ____
  Step 0:
   Item 1:
    Type: text
    Value: Fry for
   Item 2:
    Type: timer
    Name: cucumber
____           Compare Results           ____
  Test failed.
```

The last line will show whether or not the test has been passed, and if not, a message describing the problem will be displayed.

At the very end of the output will be a small summary of the full test run, which will look something like this:
```
Tests passed: x/40
Unpassed tests:
['testA','testB',...]
```
It will show how many tests have been passed, and will display an array of the tests that have not been passed.

## Making Changes:
If you would like to make your own changes to the parser itself, the parser is written in flex/bison and is in the files named _Cooklang.l_ and _Cooklang.y_, respectively. Flex handles the tokenization of the input stream, and passes the tokens to Bison, which then fits the tokens into rules. Once a rule is complete, it's callback is executed.

Flex manual can be found here: http://dinosaur.compilertools.net/flex/index.html

Bison manual can be found here: https://www.gnu.org/software/bison/manual/html_node/index.html

If you make any changes to the flex file, you will have to rebuild it. You will need to first install flex before doing so. Use the command (assuming you are in the same directory as _Cooklang.l_):
```
flex -Ca Cooklang.l
```
to do so. Please check the note at the bottom of the page before doing so.
The same goes for the bison file, which can be rebuilt by using the following command (assuming you are in the same directory as _Cooklang.y_):
```
bison Cooklang.y -d -v
```


### Working with the parser:
The yyparse function does most of the heavy lifting. It is this function that actually parses tokenized input from the flex lex.yy.c file and constructs the Recipe struct on the C side of things. The yyparse function automatically pulls input from the variable **yyin**, which can be set to a file pointer, **STDIN**, or other forms of an input stream like this:
```C
yyin = file;
```
or:
```C
yyin  = stdin;
```

A Recipe has to be initialized before calling yyparse() so that the parser function has somewhere to store all the parsed data. A Recipe struct can be initialized with these steps:
```C
// setup the recipe
Recipe * finalRecipe = createRecipe();

// create the first step and insert it
Step * currentStep = createStep();
insertBack(finalRecipe->stepList, currentStep);
```

After creating the Recipe struct and setting the input stream, the yyparse() function can be used to parse the desired input, with the created recipe as input.
```C
// parse the file
yyparse(finalRecipe);
```
The final result will a be **Recipe** structure, as defined above, containing every direction in the input.
Putting it all together would look something like this:
```C
yyin  = stdin;

// setup the recipe
Recipe * finalRecipe = createRecipe();

// create the first step and insert it
Step * currentStep = createStep();
insertBack(finalRecipe->stepList, currentStep);

// parse the file
yyparse(finalRecipe);

```


### Working with the data:

The creation of parsed data, organization, and printing of data is all handled by the .C files under the _src_ folder. There are inline comments that explain the functionality and use of functions.

The **Recipe** struct:
```C
typedef struct {
  List * metaData;
  List * stepList;

} Recipe;
```
will hold all the meta data and steps in the file after parsing.

The **Recipe** structure uses linked lists to store the parsed data. Traversal through the data after parsing is handled by the linked list library. All of the functions and documentation for the linked list can be found and described thoroughly in the _LinkedListLib.h_ file, under the _include_ folder.

For example:

The **toString(List * list)** method can be used to convert a linked list to a string, using the list's **printData** method that is supplied on creation of the recipe (the default setup is used in the **createRecipe()** function). _This is sort of like setting up a method for a class in Java._ The **printData** method for the **List * stepList** (a list of parsed steps from the input) will return a string representing the list.


The default setup uses the **stepToString(void * data)**  function.  This function returns a string which includes a separate JSON element for each direction that is found in the step, one direction per line. Each step will be separated by a empty line. At the end of the the stepList "Empty step}" will be printed to represent an empty step, which is always present after parsing a file.

If you would like to change the way that a step is printed, you can either change the **stepToString(void * data)**  function directly, or you may create your own. If you create your own, you would have to edit the **createRecipe()** function to use your function instead of the default one:
```C
// default
List * stepList = initializeList(stepToString, deleteStep, compareSteps);
tempRec->stepList = stepList;
```

```C
// new
List * stepList = initializeList(yourFunctionName, deleteStep, compareSteps);
tempRec->stepList = stepList;
```
Keep in mind that whenever you swap out a function for your own, you must follow the original function's header format for it be usable by the linked list library. The formats can be found in the LinkedListLib.h file, in the initializeList header.

If you make any changes to the .C files, or the Bison file (.y), run the following to recompile the binaries:
```
make binary_clean
make parser
// or //
make library
```



## Note:

There are many code points from the UTF-8 character database in the Cooklang.l file. This results in a very large state machine created by the flex library. As a result of this, compilation times of the executable and the shared library *can* be very long. Results may vary based on computer speeds.
