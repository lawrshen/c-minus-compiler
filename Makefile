CC = gcc
LAB2_SRC_C = main.c syntax.tab.c tree.c semantics.c symbol.c
SRC_C = main.c syntax.tab.c tree.c semantics.c symbol.c ir.c translate.c opt.c asm.c
TEST_ARGS = A

.PHONY: all debug lexical syntax clean test 

all: lexical syntax
	@$(CC) $(SRC_C) -D OPT_LAB3 -lfl -ly -o parser

debug: lexical syntax
	@$(CC) -g $(SRC_C) -D OPT_LAB3 -lfl -ly -o parser

color: lexical syntax
	@$(CC) $(SRC_C) -D COLORFUL -lfl -ly -o parser
	
ast: lexical syntax
	@$(CC) $(SRC_C) -D ASTREE -lfl -ly -o parser \
	&& ./parser w.cmm > zw.cmm

ir: lexical syntax
	@$(CC) $(SRC_C) -D OPT_LAB3 -D LAB3_ENABLE -lfl -ly -o parser \
	&& ./parser w.cmm > tmp.ir

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