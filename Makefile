.PHONY: all debug clean test

all:
	@flex lexical.l
	@bison -d syntax.y
	@gcc main.c syntax.tab.c tree.c -lfl -ly -o parser

debug:
	@flex lexical.l
	@bison -dtv syntax.y
	@gcc main.c syntax.tab.c tree.c -D YYDEBUG -lfl -ly -o parser
	
flex:
	flex lexical.l
	bison -d syntax.y
	gcc main.c syntax.tab.c tree.c -D DEBUGFLEX -D COLORFUL -lfl -ly -o parser

color:
	@flex lexical.l
	@bison -dt syntax.y
	@gcc main.c syntax.tab.c tree.c -D COLORFUL -lfl -ly -o parser

test:
	./parser test.cmm

clean:
	rm lex.yy.c syntax.tab.* parser