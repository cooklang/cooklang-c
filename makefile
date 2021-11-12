#
# Cook in C
#

all: cook cook_test

.PHONY: cook clean

cook: src/main.c
	@gcc src/main.c -o cook

cook_test: src/cook_test.c
	@gcc $< -o cook_test

test: cook_test
	@./$<

clean:
	@rm -f cook_test cook
