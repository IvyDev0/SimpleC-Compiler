result:parser.y scanner.l ast.h
	bison -d parser.y
	flex scanner.l 
	gcc parser.tab.c lex.yy.c ast.c -o parse
	rm parser.tab.c
	rm parser.tab.h
	rm lex.yy.c
	cat input.c | ./parse
	