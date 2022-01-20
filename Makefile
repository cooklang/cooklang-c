# flags for compiling a .o file
OFLAGS = -Wall -pedantic -I include/ -o $@ -g -fPIC -c

all: recompile_parser

# .o files	
bin/CooklangParser.o: src/CooklangParser.c
	gcc $(OFLAGS) src/CooklangParser.c

bin/LinkedListLib.o: src/LinkedListLib.c
	gcc $(OFLAGS) src/LinkedListLib.c

bin/CooklangRecipe.o: src/CooklangRecipe.c
	gcc $(OFLAGS) src/CooklangRecipe.c



# recompile lex file
flex:
	flex -Ca --align Cooklang.l

# parser from lex file
library: bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o
	bison -d Cooklang.y -v
	gcc -fPIC -c -g Cooklang.tab.c -lfl
	gcc -shared -o Cooklang.so bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o Cooklang.tab.o

parser: bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o
	bison -d Cooklang.y -v
	gcc -g Cooklang.tab.c -lfl bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o 


binary_clean:
	rm -f bin/*.o Cooklang.tab.o a.out Cooklang.so

# clean
full_clean: 
	rm -f bin/*.o *.o *.so test test.o lex.yy.c parser Cooklang.tab.c a.out Cooklang.tab.o lex.yy.o
