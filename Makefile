.PHONY: all debug clean test

all:
	flex lexical.l
	bison -d syntax.y
	gcc main.c syntax.tab.c -lfl -ly -o parser

debug:
	flex lexical.l
	bison -d syntax.y
	gcc main.c syntax.tab.c -D DEBUGLEVEL -lfl -ly -o parser

test:
	./cmm ./test/test1.cmm

clean:
	rm lex.yy.c syntax.tab.* parser