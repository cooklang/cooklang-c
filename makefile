#
# Cook in C
#

all: cook cook_test

.PHONY: cook clean

cook: src/main.c src/cook_flexer.c
	@gcc src/main.c -o cook

src/cook_syntax.c: src/cook.y
	bison -p cook -x -t -o $@ $<

src/cook_flexer.c: src/cook.l src/cook_syntax.c
	flex -P cook -o $@ $<

cook_test: src/cook_test.c
	@gcc $< -o cook_test

test: cook_test
	@./$<

clean:
	@rm -f cook_test cook
	@rm -f src/cook_syntax.c src/cook_syntax.xml
	@rm -f src/cook_flexer.c
