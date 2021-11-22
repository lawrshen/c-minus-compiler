CC = gcc
LAB2_SRC_C = main.c syntax.tab.c tree.c semantics.c symbol.c
SRC_C = main.c syntax.tab.c tree.c semantics.c symbol.c ir.c translate.c
TEST_ARGS = A

.PHONY: all debug lexical syntax clean test 

all: lexical syntax
	@$(CC) $(SRC_C) -lfl -ly -o parser

debug: lexical syntax
	@$(CC) -g $(SRC_C) -lfl -ly -o parser

color: lexical syntax
	@$(CC) $(SRC_C) -D COLORFUL -lfl -ly -o parser
	
ast: lexical syntax
	@$(CC) $(SRC_C) -D ASTREE -lfl -ly -o parser \
	&& ./parser w.cmm > zw.cmm

yydebug: lexical
	@bison -dtv syntax.y
	@$(CC) $(SRC_C) -D YYDEBUG -lfl -ly -o parser
	
lexical:
	@flex lexical.l

syntax:
	@bison -d syntax.y

flex_level: lexical syntax
	@gcc main.c syntax.tab.c tree.c -D DEBUGFLEX -D COLORFUL -lfl -ly -o parser

test: all
	@./test.sh $(TEST_ARGS)

clean:
	rm lex.yy.c syntax.tab.* parser