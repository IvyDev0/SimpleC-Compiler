result:parser.y scanner.l ast.h ast.c
	bison -d parser.y
	flex scanner.l 
	gcc parser.tab.c lex.yy.c ast.c -o parser	
	rm parser.tab.c parser.tab.h lex.yy.c 