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




### Via C


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
To use the library via a higher level language, the language must be installed. In this library, python was used to do the testing, with version 2.7.16. A library like *ctypes* for python can be used to interact witht the shared library file. 


To compile the code as a shared library use the commands:
```
make clean
make parser_library
```
*Please read the note at the bottom of the README before running the commands above*


While using the shared library:
-  the function 'testFile' can be used to parse a file, and returns a string which contains a JSON element for each direction in the file, one JSON element per line.
- the function 'runFile' can be used in the same way, with the same output, but the function prints the output to the command line.

