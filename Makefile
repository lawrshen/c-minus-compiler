CC = gcc
SRC_C = main.c syntax.tab.c tree.c semantics.c

.PHONY: all debug lexical syntax clean test 

all: lexical syntax
	@$(CC) $(SRC_C) -lfl -ly -o parser

color: lexical syntax
	@$(CC) $(SRC_C) -D COLORFUL -lfl -ly -o parser
	
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
	@./test.sh

clean:
	rm lex.yy.c syntax.tab.* parser