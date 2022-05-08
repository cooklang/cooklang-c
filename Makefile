# flags for compiling a .o file
OFLAGS = -Wall -pedantic -I include/ -I parserFiles/ -g -fPIC -c
OBJ=bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o

all: parser

# .o files
-include $(OBJ:.o=.d)
bin/%.o: src/%.c
	gcc $(OFLAGS) -MMD -o $@ $<

# recompile lex file
lex.yy.c: src/Cooklang.l
	flex -Ca -o parserFiles/lex.yy.c $<

flex: lex.yy.c

# recompile bison file
parserFiles/Cooklang.tab.c: src/Cooklang.y
	bison $< -d -v -o parserFiles/Cooklang.tab.c

bison: parserFiles/Cooklang.tab.c

# make shared library parser file
library: parserFiles/Cooklang.tab.c $(OBJ)
	gcc -fPIC -DLIB -c -g -o bin/Cooklang.tab.o parserFiles/Cooklang.tab.c
	gcc -shared -o bin/Cooklang.so $(OBJ) bin/Cooklang.tab.o
	$(info Created in 'bin/Cooklang.so')


# make executable parser
parser: parserFiles/Cooklang.tab.c $(OBJ) parserFiles/lex.yy.c
	gcc -g parserFiles/$< $(OBJ) -o $@

# clean binaries
binary_clean:
	rm -rf bin/*.o *.o *.so *.out parser build/*

# clean everything
full_clean: binary_clean
	rm -rf bin/*.d test parserFiles/*
