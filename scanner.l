%option noyywrap yylineno
%{
#include "ast.h"
#include "parser.tab.h" 
%}

TYPE int|float
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
PLUS \+
MINUS -
INTEGER [1-9]+[0-9]*|0 
FLOAT   [0-9]+\.[0-9]*  
OCT			[0][1-7]+[0-7]*
HEX			[0][Xx][0-9A-Fa-f]*
ID [a-z_A-Z][a-zA-Z_0-9]*
SPACE [ \t\r]*
EOL \n
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}

AERROR .
%%
int|float {yylval.a=newnode("TYPE",0,yylineno); return TYPE;}
struct {yylval.a=newnode("STRUCT",0,yylineno);return STRUCT;}
{RETURN} {yylval.a=newnode("RETURN",0,yylineno); return RETURN;}
{IF} { yylval.a=newnode("IF",0,yylineno);return IF;}
{ELSE} {yylval.a=newnode("ELSE",0,yylineno); return ELSE;}
{WHILE} {yylval.a=newnode("WHILE",0,yylineno); return WHILE;}
{PLUS} {yylval.a=newnode("PLUS",0,yylineno); return PLUS;}
{MINUS} {yylval.a=newnode("MINUS",0,yylineno); return MINUS;}
{INTEGER} {yylval.a=newnode("INTEGER",0,yylineno); return INTEGER;}
{FLOAT} {yylval.a=newnode("FLOAT",0,yylineno); return INTEGER;}
{OCT} {yylval.a=newnode("OCT",0,yylineno); return INTEGER;}
{HEX} {yylval.a=newnode("HEX",0,yylineno); return INTEGER;}
{ID} {yylval.a=newnode("ID",0,yylineno); return ID;}
{SPACE} {}
{EOL} {}
{SEMI} {yylval.a=newnode("SEMI",0,yylineno); return SEMI;}
{COMMA} {yylval.a=newnode("COMMA",0,yylineno); return COMMA;}
{ASSIGNOP} {yylval.a=newnode("ASSIGNOP",0,yylineno); return ASSIGNOP;}
{RELOP} {yylval.a=newnode("RELOP",0,yylineno); return RELOP;}
{STAR} {yylval.a=newnode("STAR",0,yylineno); return STAR;}
{DIV} {yylval.a=newnode("DIV",0,yylineno); return DIV;}
{AND} {yylval.a=newnode("AND",0,yylineno); return AND;}
{OR} {yylval.a=newnode("OR",0,yylineno); return OR;}
{DOT} {yylval.a=newnode("DOT",0,yylineno); return DOT;}
{NOT} {yylval.a=newnode("NOT",0,yylineno); return NOT;}
{LP} {yylval.a=newnode("LP",0,yylineno); return LP;}
{RP} {yylval.a=newnode("RP",0,yylineno); return RP;}
{LB} {yylval.a=newnode("LB",0,yylineno); return LB;}
{RB} {yylval.a=newnode("RB",0,yylineno); return RB;}
{LC} {yylval.a=newnode("LC",0,yylineno); return LC;}
{RC} {yylval.a=newnode("RC",0,yylineno); return RC;}
\/\/[^\n]*  {}  /* line annotation */
"/*"([^\*]|\*[^/])*"*/"  {}  /* block annotation */
"/*"([^\*]|\*[^/])*$  {printf("line %d: error: undeterminated comment\n",yylineno);}
"*/"  {printf("line %d: error: undeterminated comment\n",yylineno);}


{AERROR} { printf("Error type A at line %d: Mystirious charachter '%s'\n",yylineno,yytext);}

%%
