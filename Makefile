# flags for compiling a .o file
OFLAGS = -Wall -std=c99 -pedantic -I include/ -o $@ -c -g
FLAGS = -Wall -std=c99 -pedantic -o $@ -g

all: test

parser: parser

# executables
test: test.o bin/CooklangParser.o bin/LinkedListLib.o
	gcc $(FLAGS) test.o bin/CooklangParser.o bin/LinkedListLib.o
	

# .o files	
bin/CooklangParser.o: src/CooklangParser.c
	gcc $(OFLAGS) src/CooklangParser.c

bin/LinkedListLib.o: src/LinkedListLib.c
	gcc $(OFLAGS) src/LinkedListLib.c


test.o:
	gcc $(OFLAGS) test.c

# parser stuff
parser:
	flex Cooklang.l
	bison -d Cooklang.y -v
	gcc Cooklang.tab.c -lfl


# clean
clean: 
	rm bin/*.o *.o test test.o lex.yy.c parser Cooklang.tab.c a.out Cooklang.tab.o lex.yy.o