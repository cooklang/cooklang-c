# flags for compiling a .o file
OFLAGS = -Wall -pedantic -I include/ -I parserFiles/ -g -fPIC -c
OBJ=bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o

all: parser

# .o files
-include $(OBJ:.o=.d)
bin/%.o: src/%.c
	gcc $(OFLAGS) -MMD -o $@ $<

# recompile lex file
lex.yy.c: parserFiles/Cooklang.l
	flex -Ca $< -o parserFiles

flex: lex.yy.c

# recompile bison file
Cooklang.tab.c: Cooklang.y
	bison $< -d -v -o parserFiles/Cooklang.tab.c

bison: Cooklang.tab.c

# make shared library parser file
library: Cooklang.tab.c $(OBJ)
	gcc -fPIC -DLIB -c -g Cooklang.tab.c
	gcc -shared -o Cooklang.so $(OBJ) Cooklang.tab.o


# make executable parser
parser: Cooklang.tab.c $(OBJ)
	gcc -g parserFiles/$< $(OBJ) -o $@

# clean binaries
binary_clean:
	rm -f bin/*.o *.o *.so *.out parser

# clean everything
full_clean: binary_clean
	rm -f bin/*.d  test parserFiles/Cooklang.tab.c parserFiles/Cooklang.tab.h parserFiles/Cooklang.output
