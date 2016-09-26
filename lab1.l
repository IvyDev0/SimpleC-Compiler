%option yylineno
%{
#include <stdio.h>
%}

/* definitions */

digit [0-9]
number {digit}+
letter [A-Za-z]
es	[eE]+
floate	-?[1-9]+(\.{number})?{es}-?[1-9]+(\.{number})?
floaterror1	-?{number}(\.{number})?{es}
floaterror2	{es}-?{number}(\.{number})?


ID      {letter}({letter}|{digit})*
INT		^-?[1-9]\d*$
FLOAT	^-?([1-9]\d*\.\d*|0\.\d*[1-9]\d*|0?\.0+|0)$

SEMI	;
COMMA	,
ASSIGNOP	=
RELOP	> | < | <= | >= | == | !=
PLUS	+
MINUS	-
STAR	*
DIV		/
AND		&&
OR		||
DOT		.
NOT		!
TYPE	int | float
LP		(
RP      )
LB		[
RB      ]
LC		{
RC		}
STRUCT	struct
RETURN	return
IF		if
ELSE	else
WHILE	while


%%  

[ \t]+ {}
\n {}
{letter} {}
[1-9]+[0-9]* {} /* numbers except for beginning with 0 */
if|then|else|end|repeat|until|read|write|or|and|int|bool|char|while|do {}  /* keyword */

"{"|"}"|":="|"_"|"="|"<"|"+"|"-"|"/"|"("|"*"|")"|";"|"<="|">="|">"|"," {}     

{floate} 	{printf("E-float: %s\n", yytext);}

{floaterror1} {
char *p = yytext;
if(!(p[0]=='0'&&(p[1]=='x'||p[1]=='X')))
printf("Error type [A] at Line[%d] : There should be a number AFTER e/E\n",yylineno);
}

{floaterror2} {
char *p = yytext;
if(!(p[0]=='0'&&(p[1]=='x'||p[1]=='X')))
printf("Error type [A] at Line[%d] : There should be a number BEFORE e/E\n",yylineno);
}


.	{
printf("Eorro type [A] at line %d: Unrecognized character '%s'\n", yylineno, yytext);

}



%%  
/* user subroutines */

int main () {
	yylex();
	return 0;	
}

int yywrap() {
	return 1;
}
