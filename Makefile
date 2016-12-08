result:tree.y scanner.l ast.h ast.c
	bison -d tree.y
	flex scanner.l 
	gcc tree.tab.c lex.yy.c ast.c -o parse	
	rm tree.tab.c tree.tab.h lex.yy.c 