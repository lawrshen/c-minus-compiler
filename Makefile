.PHONY: all debug clean test

all:
	@flex lexical.l
	@bison -d syntax.y
	@gcc main.c syntax.tab.c tree.c -lfl -ly -o parser
	@./parser test.cmm

debug:
	@flex lexical.l
	@bison -dt syntax.y
	@gcc main.c syntax.tab.c tree.c -D YYDEBUG -lfl -ly -o parser
	@./parser test.cmm
	
flex:
	flex lexical.l
	bison -d syntax.y
	gcc main.c syntax.tab.c tree.c -D DEBUGFLEX -lfl -ly -o parser
	./parser test.cmm

test:
	./parser w.cmm

clean:
	rm lex.yy.c syntax.tab.* parser