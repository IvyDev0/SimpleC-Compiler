result:parser.y scanner.l ast.h
	bison -d parser.y
	flex scanner.l 
	gcc parser.tab.c lex.yy.c ast.c -o scanner