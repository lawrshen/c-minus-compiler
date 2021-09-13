.PHONY: all debug clean test

all:
	flex lexical.l
	bison -d syntax.y
	gcc main.c syntax.tab.c -lfl -ly -o parser
	./parser w.cmm

debug:
	flex lexical.l
	bison -d syntax.y
	gcc main.c syntax.tab.c -D DEBUGLEVEL -lfl -ly -o parser
	./parser w.cmm
	
test:
	./parser w.cmm

clean:
	rm lex.yy.c syntax.tab.* parser