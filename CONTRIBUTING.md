# Contributing
If you are using the library, and find any problems, or even have any suggestions, please open an issue on the github and we can work towards a solution.

## How to contribute:
If you would like to work with the library you will need the following packages installed: flex, bison, and make and gcc for the compilation. These can all be installed via:
```
sudo apt-get install flex bison make gcc
```

The library is built using these versions:
```
flex 2.6.4
bison (GNU Bison) 3.3.2
GNU Make 4.2.1
gcc (Debian 8.3.0-6) 8.3.0
```




### via C


To compile the code as an executable use the commands:
```
make clean
make parser
```
*Please read the note at the bottom of the README before running the commands above*

This will compile the executable into a file with the default name './a.out'.
While using the executable file, the desired file can be chosen to be parsed by writing as the first argument(without the quotations):

```
./a.out 'fileName.cook'
```
The output will be printed to stdout.


### via Python/other
To use the library via a higher level language, the language must be installed. In this library, python was used to do the testing, with version 2.7.16. A library like *ctypes* for python can be used to interact witht the shared library file. The user must also be able to run C executables.


To compile the code as a shared library use the commands:
```
make clean
make parser_library
```
*Please read the note at the bottom of the README before running the commands above*


While using the shared library:
-  the function 'testFile' can be used to parse a file, and returns a string which contains a JSON element for each direction in the file, one JSON element per line.
- the function 'runFile' can be used in the same way, with the same output, but the function prints the output to the command line.



## Testing:
To test the parser against some of your own test cases or files, the _testing.py_ file under the _testing_ folder can be used. It is written in Python and uses **ctypes** to interface with the shared object file. 

1. The script takes input tests in JSON format. The JSON format is the same as the testing format described in the _canonical.yaml_ file in the Cooklang spec under _spec/tests/canonical.yaml_. It is simple to convert the yaml to JSON using an online yaml to JSON converter. Have the tests to run in the _test.json_ file using the correct format.

2. Follow the steps above under **_via Python/other_** to compile the shared object file. After it has completed, move the Cooklang.so file into the _testing_ folder, the same directory as the _testing.py_ file is in. The user will have to update the value of **so** in the _testing.py_ file to the absolute path to the shared object file, which might look something like:
```
/mnt/c/Users/userName/Documents/cook-in-c/testing/Cooklang.so
```

3. To run the script simply type the following into a command line in the same directory as the _testing.py_ file:
```
python testing.py
```

The output will be printed to the command line, where each test will look something like this:
```
____  Test: testSlashInText
____           Expected Output           ____
{ "type": "text", "value": "Preheat the oven to 200℃/Fan 180°C."}
____            Actual Output            ____
{ "type": "text", "value": "Preheat the oven to 200℃/Fan 180°C." }
____           Compare Results           ____
  Test Passed.
```

The last line will show whether or not the test has been passed, and if not, a message describing the problem will be displayed.

At the very end of the output will be a small summary of the full test run, which will look something like this:
```
Tests passed: x/40
Unpassed tests:
['testA','testB',...]
```
It will show how many tests have been passed, and will display an array of the tests that have not been passed.

### Note:
The tests will not be completed in any specific order, regardless of the order in the _tests.json_ file, since Python uses unordered dictionaries.











































