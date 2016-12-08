%{
#include "ast.h"
%}
%union{
struct ast* a;
double d;
}
/*declare tokens*/
%token  <a> INTEGER FLOAT
%token <a> TYPE STRUCT RETURN IF ELSE WHILE ID SPACE SEMI COMMA ASSIGNOP RELOP
PLUS
MINUS STAR DIV AND OR DOT NOT LP RP LB RB LC RC AERROR
%token <a> EOL
%type  <a> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier
OptTag  Tag VarDec  FunDec VarList ParamDec Compst StmtList Stmt DefList Def
DecList Dec Exp Args
/*priority*/
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT
%%
Program:ExtDefList {
	$$=newast("Program",1,$1);
	printf("Print syntaxtree:\n");eval($$,0);printf("Finish printing.\n\n");
	}
    ;
ExtDefList:ExtDef ExtDefList {$$=newast("ExtDefList",2,$1,$2);}
    | {$$=newast("ExtDefList",0,-1);}
    ;
ExtDef:Specifier ExtDecList SEMI {
		$$=newast("ExtDef",3,$1,$2,$3);
		if(existvar($2)) // 3 - 检查变量是否重复定义
	        printf("Error type 3 at Line %d:Redefined Variable '%s'\n",yylineno,$2->content);
	    else 
	    	newvar(2,$1,$2);
	}
    |Specifier SEMI {$$=newast("ExtDef",2,$1,$2);}
    |Specifier FunDec Compst    {
    	$$=newast("ExtDef",3,$1,$2,$3);
    	newfunc(4,$1);
    }
    ;
ExtDecList:Dec {$$=newast("ExtDecList",1,$1);}
    |Dec COMMA ExtDecList {$$=newast("ExtDecList",3,$1,$2,$3);}
    ;
/*Specifier*/
Specifier:TYPE {$$=newast("Specifier",1,$1);}
    |StructSpecifier {$$=newast("Specifier",1,$1);}
    ;
StructSpecifier:STRUCT OptTag LC DefList RC {
		$$=newast("StructSpecifier",5,$1,$2,$3,$4,$5);
	    if(existstruc($2))   
	        printf("Error type 16 at Line %d:Duplicated structure name '%s'\n",yylineno,$2->content);
	    else newstruc(1,$2);
	}
    |STRUCT Tag {
	    $$=newast("StructSpecifire",2,$1,$2);
	    if(!existstruc($2)) printf("Error type 17 at Line %d:Undefined structure '%s'\n",yylineno,$2->content);
    }
    ;
OptTag:ID {$$=newast("OptTag",1,$1);}
    |{$$=newast("OptTag",0,-1);}
    ;
Tag:ID {$$=newast("Tag",1,$1);}
	;
/*Declarators*/
VarDec:ID {printf("VarDec:ID ----- %s\n",$1->content);$$=newast("VarDec",1,$1);$$->tag=1;}
    |VarDec LB INTEGER RB {$$=newast("VarDec",4,$1,$2,$3,$4);$$->content=$1->content;$$->tag=4;}
    ;
FunDec:ID LP VarList RP {
	$$=newast("FunDec",4,$1,$2,$3,$4);
    if(existfunc($1)) // 4 - 函数重复定义 
        printf("Error type 4 at Line %d:Redefined Function '%s'\n",yylineno,$1->content);
    else newfunc(2,$1);
    }
    |ID LP RP 
    {
        $$=newast("FunDec",3,$1,$2,$3);
        if(existfunc($1)) // 4 - 函数重复定义 
            printf("Error type 4 at Line %d:Redefined Function '%s'\n",yylineno,$1->content);
        else newfunc(2,$1);
    }
    ;
VarList:ParamDec COMMA VarList {$$=newast("VarList",3,$1,$2,$3);}
    |ParamDec {$$=newast("VarList",1,$1);}
    ;
ParamDec:Specifier VarDec {$$=newast("ParamDec",2,$1,$2);newfunc(1);}
    ;
/*Statement*/
Compst:LC DefList StmtList RC {$$=newast("Compst",4,$1,$2,$3,$4);}
    ;


StmtList:Stmt StmtList{$$=newast("StmtList",2,$1,$2);}
    | {$$=newast("StmtList",0,-1);}
    ;
Stmt:Exp SEMI {printf("Stmt:Exp SEMI ------ \n");$$=newast("Stmt",2,$1,$2);}
    |Compst {$$=newast("Stmt",1,$1);}
    |RETURN Exp SEMI {$$=newast("Stmt",3,$1,$2,$3);newfunc(3,$2)}
    |IF LP Exp RP Stmt {$$=newast("Stmt",5,$1,$2,$3,$4,$5);}
    |IF LP Exp RP Stmt ELSE Stmt {$$=newast("Stmt",7,$1,$2,$3,$4,$5,$6,$7);}
    |WHILE LP Exp RP Stmt {$$=newast("Stmt",5,$1,$2,$3,$4,$5);}
    ;
/*Local Definitions*/
DefList:Def DefList{$$=newast("DefList",2,$1,$2);}
    | {$$=newast("DefList",0,-1);}
    ;
Def:Specifier DecList SEMI 
	{
        $$=newast("Def",3,$1,$2,$3);
        if(existvar($2)||existarray($2)) // 3 - 检查变量是否重复定义
        	printf("Error type 3 at Line %d:Redefined Variable '%s'\n",yylineno,$2->content);
    	else if($2->tag==4) newarray(2,$1,$2);
    	else  newvar(2,$1,$2); 
    }
    ;
DecList:Dec {$$=newast("DecList",1,$1);}
    |Dec COMMA DecList {$$=newast("DecList",3,$1,$2,$3);$$->tag=$3->tag;}
	;
Dec:VarDec {$$=newast("Dec",1,$1);}
	|VarDec ASSIGNOP Exp {printf("VarDec ASSIGNOP Exp\n");$$=newast("Dec",3,$1,$2,$3);$$->content=$1->content;}
    ;
/*Expressions*/
Exp:Exp ASSIGNOP Exp {
        $$=newast("Exp",3,$1,$2,$3);
        printf("Exp ASSIGNOP Exp:%s,%s\n", $1->type, $3->type);
        if($1->tag==3) // 6 - 检查赋值号左边是否出现右值
            printf("Error type 6 at Line %d:Illegal assignment.\n ",yylineno);

        else if(strcmp($1->type,$3->type)) // 5 - 检查等号左右类型匹配判断
            printf("Error type 5 at Line %d:Type mismatched for assignment.\n ",yylineno);

        else {$$->content = $2->content; $$->value = $2->value;}

        /*try{ 
            int add = &($1);  // 6 - 检查赋值号左边是否出现右值。左值可以取地址，右值不可以。
        } catch() {
            printf("Error type 6 at Line %d:Illegal assignment.\n ",yylineno);
        }*/
    }
    |Exp AND Exp{$$=newast("Exp",3,$1,$2,$3);}
    |Exp PLUS Exp {
        $$=newast("Exp",3,$1,$2,$3);
        if(strcmp($1->type,$3->type)) // 7 - 检查操作符左右类型
            printf("Error type 7 at Line %d:Type mismatched for operand.\n ",yylineno);
        else { $$->type = $1->type; $$->tag = 3; }
        //else {$$->content = $1->content + $2->content; $$->value = $1->value + $2->value;}
    }
    |Exp STAR Exp {
        $$=newast("Exp",3,$1,$2,$3); // 7 - 检查操作符左右类型
        if(strcmp($1->type,$3->type))
            printf("Error type 7 at Line %d:Type mismatched for operand.\n ",yylineno);
        //else {$$->content = $1->content * $2->content; $$->value = $1->value * $2->value;}
    }
    |Exp DIV Exp {
        $$=newast("Exp",3,$1,$2,$3); // 7 - 检查操作符左右类型
        if(strcmp($1->type,$3->type)) printf("Error type 7 at Line %d:Type mismatched for operand.\n ",yylineno);
    }
    |LP Exp RP{$$=newast("Exp",3,$1,$2,$3);}
    |MINUS Exp {$$=newast("Exp",2,$1,$2);}
    |NOT Exp {$$=newast("Exp",2,$1,$2);}
    |ID LP Args RP {
        $$=newast("Exp",4,$1,$2,$3,$4);
        if(!existfunc($1)) // 函数引用:检查是否未定义就调用Error type 2 
            printf("Error type 2 at Line %d:Undefined function %s\n ",yylineno,$1->content);
        else if(!pnumfunc($1)) // 9 - 函数调用时实参与形参的数目不匹配 
            printf("Error type 9 at Line %d:Number of parameters mismatched.\n ",yylineno);

    }
    |ID LP RP {
    	$$=newast("Exp",3,$1,$2,$3);
    	if(!existfunc($1)) // 11- 对普通变量使用了函数调用的操作符 
    		printf("Error type 11 at Line %d:Illegal call %s()\n ",yylineno,$1->content);
    	else if(pnumfunc($1)!=0) // 9 - 函数调用时实参与形参的数目不匹配 
            printf("Error type 9 at Line %d:Number of parameters mismatched.\n ",yylineno);
    }
    |Exp LB Exp RB {
    	printf("Exp LB Exp RB: %s,%s\n", $1->content,$1->type);
        $$=newast("Exp",4,$1,$2,$3,$4);

        if((!existarray($1))&&(existvar($1)||existfunc($1))) // 10 - 对非数组类型使用了[]数组访问的操作符 
            printf("Error type 10 at Line %d:'%s'is not an array.\n ",yylineno,$1->content);
        else if(!existarray($1))
            printf("Error type 2 at Line %d:Undefined Array %s\n ",yylineno,$1->content);
    	else if(!strcmp($3->type,"int")) //数组访问下标出现了非整数 Error type 12
            printf("Error type 12 at Line %d:%.1f is not a integer.\n",yylineno,$3->value);
        else 
        	$$->type = $1->type;
        }
    |Exp DOT ID {
        $$=newast("Exp",3,$1,$2,$3);
        printf("Exp DOT ID ： %s\n", $1->type);
        if(!existstruc($1)) //结构体引用:检查点号引用 Error type 13
            printf("Error type 13 at Line %d:Illegal use of '.'.\n",yylineno);
    }
    |ID {
        $$=newast("Exp",1,$1);
    	printf("%s\n", $1->content);
    	if(existvar($1)) { $$->type=typevar($1); $$->content = $1->content; }
    	else if(existarray($1)) $$->type=typearray($1);
    	else printf("Error type 1 at Line %d:Undefined variable %s\n ",yylineno,$1->content);
	}
    |INTEGER {$$=newast("Exp",1,$1);$$->value=$1->value;$$->tag=3;$$->type="int";} //整型常数
    |FLOAT{$$=newast("Exp",1,$1);$$->tag=3;$$->type="float";$$->value=$1->value;} //浮点型常数
    ;
Args:Exp COMMA Args {$$=newast("Args",3,$1,$2,$3);rpnum+=1;}
    |Exp {$$=newast("Args",1,$1);rpnum+=1;}
    ;
%%
