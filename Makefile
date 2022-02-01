# flags for compiling a .o file
OFLAGS = -Wall -pedantic -I include/ -g -fPIC -c
OBJ=bin/CooklangParser.o bin/CooklangRecipe.o bin/LinkedListLib.o

all: parser

# .o files
-include $(OBJ:.o=.d)
bin/%.o: src/%.c
	gcc $(OFLAGS) -MMD -o $@ $<

# recompile lex file
lex.yy.c: Cooklang.l
	flex -Ca $<

flex: lex.yy.c

Cooklang.tab.c: Cooklang.y
	bison -d  $< -v

bison: Cooklang.tab.c

# parser from lex file
library: Cooklang.tab.c $(OBJ)
	gcc -fPIC -DLIB -c -g Cooklang.tab.c -lfl
	gcc -shared -o Cooklang.so $(OBJ) Cooklang.tab.o

parser: Cooklang.tab.c $(OBJ)
	gcc -g $< -lfl $(OBJ) -o $@


binary_clean:
	rm -f bin/*.o *.o *.so *.out parser

# clean
full_clean: binary_clean
	rm -f bin/*.d  test Cooklang.tab.c lex.yy.c
