# flags for compiling a .o file
OFLAGS = -Wall -pedantic -I include/ -o $@ -c -g
FLAGS = -Wall -pedantic -o $@ -g

all: test

parser: parser

# executables
test: test.o bin/CooklangRecipe.o bin/CooklangParser.o bin/LinkedListLib.o 
	gcc $(FLAGS) test.o bin/CooklangRecipe.o bin/CooklangParser.o bin/LinkedListLib.o
	

# .o files	
bin/CooklangParser.o: src/CooklangParser.c
	gcc $(OFLAGS) src/CooklangParser.c

bin/LinkedListLib.o: src/LinkedListLib.c
	gcc $(OFLAGS) src/LinkedListLib.c

bin/CooklangRecipe.o: src/CooklangRecipe.c
	gcc $(OFLAGS) src/CooklangRecipe.c


test.o:
	gcc $(OFLAGS) test.c

# parser stuff
parser: bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o
	flex -Ca --align Cooklang.l
	bison -d Cooklang.y -v
	gcc -g Cooklang.tab.c -lfl bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o


# clean
clean: 
	rm -f bin/*.o *.o test test.o lex.yy.c parser Cooklang.tab.c a.out Cooklang.tab.o lex.yy.o
