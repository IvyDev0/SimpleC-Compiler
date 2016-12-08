result:parse.y scanner.l ast.h ast.c
	bison -d parse.y
	flex scanner.l 
	gcc parse.tab.c lex.yy.c ast.c -o parse	
	rm parse.tab.c parse.tab.h lex.yy.c 