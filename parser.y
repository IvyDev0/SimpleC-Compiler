%{
#include "ST.h"
%}
%union {
struct astnode* a;
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
    // 注：
    // - newsymbol参数为两个：类型（先检查此结点nodetag是否为5，结构体对象）、名字（此节点nodetag：2为普通变量，3为数组，4为结构体，6为函数）
    // - 构造结点的newnode中的行为 —— 第一个语法单元为左结点，其content值、nodetag值、行号传给父节点；之后的兄弟结点作为右结点连接。
    // - newsymbol函数中包括了检查是否已定义
    // - nodetag：-1为纯右值表达式, 0为int数字，1为float数字，2为一般变量，3为数组，4为结构体，5为结构体对象，6为函数
    // - (TODO: ExtDecList 的连接问题)
    
    	$$=newnode("Program",1,$1);
    	printf("Print syntaxtree:\n");
        eval($$,0); 
        printf("Finish printing.\n\n");
        clearast($$); 
        cleartable();
	}
    ;
ExtDefList:ExtDef ExtDefList { $$=newnode("ExtDefList",2,$1,$2); }
    | {$$=newnode("ExtDefList",0,-1);}
    ;
ExtDef:Specifier ExtDecList SEMI {
		$$=newnode("ExtDef",3,$1,$2,$3);
	    newsymbol($1,$2); 
	}
    |Specifier SEMI { //这表示只有结构体声明时
        $$=newnode("ExtDef",2,$1,$2); 
    }
    |Specifier FunDec Compst {
    	$$=newnode("ExtDef",3,$1,$2,$3);
    	newsymbol($1,$2); // 这里$2 - FunDec里要含有：函数名、参数个数
    }
    ;
ExtDecList:Dec {$$=newnode("ExtDecList",1,$1);$$->value = $1->value;}
    |Dec COMMA ExtDecList {
        $$=newnode("ExtDecList",3,$1,$2,$3);
    }
    ;
/*Specifier*/
Specifier:TYPE { 
        $$=newnode("Specifier",1,$1); 
        printf("Specifier:%s\n", $$->content[0]);
    }
    |StructSpecifier { $$=newnode("Specifier",1,$1); }
    ;
StructSpecifier:STRUCT OptTag LC DefList RC {
		$$=newnode("StructSpecifier",5,$1,$2,$3,$4,$5);
        $$->content[0] = $2->content[0];
        $$->nodetag = 5;
        $2->nodetag = 4;
        newsymbol($1,$2);
	}
    |STRUCT Tag {
	    $$=newnode("StructSpecifier",2,$1,$2);
        $$->content[0] = $2->content[0];
        $$->nodetag = 5;
	    if(!havedefined($2)) 
            printf("Error type 17 at Line %d:Undefined structure '%s'\n",yylineno,$2->content[0]);
    }
    ;
OptTag:ID {$$=newnode("OptTag",1,$1);}
    |{$$=newnode("OptTag",0,-1);}
    ;
Tag:ID {$$=newnode("Tag",1,$1);}
	;
/*Declarators*/
FunDec:ID LP VarList RP {       
        $$=newnode("FunDec",4,$1,$2,$3,$4); 
        $$->value = $3->value; // 形参个数
        $$->nodetag = 6;
    }
    |ID LP RP 
    {
        $$=newnode("FunDec",3,$1,$2,$3);
        $$->value = 0;
        $$->nodetag = 6;

        printf("FunDec: %s\n", $1->content[0]);
    }  
    ;
VarList:ParamDec COMMA VarList { 
        $$=newnode("VarList",3,$1,$2,$3); 
        $$->value = $1->value + $3->value; // VarList即形参表，这里形参个数累加
    }
    |ParamDec { $$=newnode("VarList",1,$1); $$->value = $1->value; }
    ;
ParamDec:Specifier VarDec {
        $$=newnode("ParamDec",2,$1,$2);
        $$->value = 1; 
    }
    ;
VarDec:ID {
        $$ = newnode("VarDec",1,$1); 
        $$->nodetag = 2;
        
        printf("VarDec:%s\n", $$->content[0]); 
    }
    |VarDec LB INTEGER RB {
        $$ = newnode("VarDec",4,$1,$2,$3,$4);
        $$->nodetag = 3;
    }
    ;
/*Statement*/
Compst:LC DefList StmtList RC { $$=newnode("Compst",4,$1,$2,$3,$4); }
    ;
StmtList:Stmt StmtList{ $$=newnode("StmtList",2,$1,$2); }
    | { $$=newnode("StmtList",0,-1); }
    ;
Stmt:Exp SEMI {$$=newnode("Stmt",2,$1,$2);}
    |Compst {$$=newnode("Stmt",1,$1);}
    |RETURN Exp SEMI {
        $$=newnode("Stmt",3,$1,$2,$3);
        checkreturn($2); // 找到最后一个函数，检查nodetag是否相同
    }
    |IF LP Exp RP Stmt { $$=newnode("Stmt",5,$1,$2,$3,$4,$5); }
    |IF LP Exp RP Stmt ELSE Stmt { $$=newnode("Stmt",7,$1,$2,$3,$4,$5,$6,$7); }
    |WHILE LP Exp RP Stmt { $$=newnode("Stmt",5,$1,$2,$3,$4,$5); }
    ;
/*Local Definitions*/
DefList:Def DefList{ $$=newnode("DefList",2,$1,$2); }
    | { $$=newnode("DefList",0,-1); }
    ;
Def:Specifier DecList SEMI 
	{
        $$=newnode("Def",3,$1,$2,$3);
        newsymbol($1,$2); 
    }
    ;
DecList:Dec { 
        $$=newnode("DecList",1,$1); 
        $$->value = $1->value;
    }
    |Dec COMMA DecList { $$=newnode("DecList",3,$1,$2,$3); }
	;
Dec:VarDec { printf("Dec - VarDec:%s\n", $1->content[0]); $$=newnode("Dec",1,$1); }
	|VarDec ASSIGNOP Exp { 
        $$=newnode("Dec",3,$1,$2,$3); 
        $$->value = $3->value; 
    }
    ;
/*Expressions*/
Exp:Exp ASSIGNOP Exp {
        $$=newnode("Exp",3,$1,$2,$3);
        $$->nodetag = -1;
        if($1->nodetag == -1) // 6 - 检查赋值号左边是否出现右值
            printf("Error type 6 at Line %d:Illegal assignment.\n ",yylineno);
        else if((!strcmp($1->l->s->type,"int") && $3->nodetag==1) || (!strcmp($1->l->s->type,"float") && $3->nodetag==0)) // 5 - 检查等号左右类型匹配判断
            printf("Error type 5 at Line %d:Type mismatched for assignment.\n ",yylineno);
        else $1 = $2;
    }
    |Exp AND Exp{
        $$=newnode("Exp",3,$1,$2,$3);
        $$->nodetag = -1;
    }
    |Exp PLUS Exp {
        $$=newnode("Exp",3,$1,$2,$3);
        $$->nodetag = -1;
        if((!strcmp($1->l->s->type,"int") && $3->nodetag==1) || (!strcmp($1->l->s->type,"float") && $3->nodetag==0)) // 5 - 检查等号左右类型匹配判断
            printf("Error type 7 at Line %d:Type mismatched for operand.\n ",yylineno);
        else $$->value = $1->value + $2->value;
    }
    |Exp STAR Exp {
        $$=newnode("Exp",3,$1,$2,$3); 
        $$->nodetag = -1;
        if((!strcmp($1->l->s->type,"int") && $3->nodetag==1) || (!strcmp($1->l->s->type,"float") && $3->nodetag==0)) // 5 - 检查等号左右类型匹配判断
            printf("Error type 7 at Line %d:Type mismatched for operand.\n ",yylineno);
        else $$->value = $1->value * $2->value;
    }
    |Exp DIV Exp {
        $$=newnode("Exp",3,$1,$2,$3); 
        $$->nodetag = -1;
        if((!strcmp($1->l->s->type,"int") && $3->nodetag==1) || (!strcmp($1->l->s->type,"float") && $3->nodetag==0)) // 5 - 检查等号左右类型匹配判断
            printf("Error type 7 at Line %d:Type mismatched for operand.\n ",yylineno);
        else $$->value = $1->value / $2->value;
    }
    |LP Exp RP{ $$=newnode("Exp",3,$1,$2,$3); }
    |MINUS Exp { $$=newnode("Exp",2,$1,$2); }
    |NOT Exp { $$=newnode("Exp",2,$1,$2); }
    |ID LP Args RP {
        $$=newnode("Exp",4,$1,$2,$3,$4);
        if(!havedefined($1)) // 函数引用:检查是否未定义就调用Error type 2 
            printf("Error type 2 at Line %d:Undefined function %s\n ",yylineno,$1->content[0]);
        else if(getdefined($1) == 6 && $1->s->pnum!=rpnum) // 9 - 函数调用时实参与形参的数目不匹配 
            printf("Error type 9 at Line %d:Number of parameters mismatched.\n ",yylineno);
        rpnum = 0;
    }
    |ID LP RP {
    	$$=newnode("Exp",3,$1,$2,$3);
    	if(getdefined($1) == 2) // 11- 对普通变量使用了函数调用的操作符 
    		printf("Error type 11 at Line %d:Illegal call %s()\n ",yylineno,$1->content[0]);
    	else if($1->s->pnum!=rpnum) 
            printf("Error type 9 at Line %d:Number of parameters mismatched.\n ",yylineno);
    }
    |Exp LB Exp RB {
        $$=newnode("Exp",4,$1,$2,$3,$4);
        if(getdefined($1) != 3) // 10 - 对非数组类型使用了[]数组访问的操作符 
            printf("Error type 10 at Line %d:'%s'is not an array.\n ",yylineno,$1->content[0]);
        else if(!havedefined($1))
            printf("Error type 2 at Line %d:Undefined Array %s\n ",yylineno,$1->content[0]);
    	else if(!strcmp($3->l->gramname,"INTEGER")) //数组访问下标出现了非整数 Error type 12
            printf("Error type 12 at Line %d:%.1f is not a integer.\n",yylineno,$3->value);
        else 
            $$->nodetag = 3;
        }
    |Exp DOT ID {
        $$=newnode("Exp",3,$1,$2,$3);
        if(getdefined($1) != 5) // Error type 13 对非结构体变量使用了.操作
            printf("Error type 13 at Line %d:Illegal use of '.'.\n",yylineno);
    }
    |ID {
        $$=newnode("Exp",1,$1);
        $$->nodetag = 2;
    	if(!havedefined($1)) 
    	   printf("Error type 1 at Line %d:Undefined variable %s\n ",yylineno,$1->content[0]);

        printf("ID:%s\n", $1->content[0]);
	}
    |INTEGER {
        $$=newnode("Exp",1,$1);
        $$->value=$1->value;
        $$->nodetag = 0;

        printf("EXP - INTEGER:%s\n", $1->content[0]);
    } 
    |FLOAT {
        $$=newnode("Exp",1,$1);
        $$->value=$1->value;
        $$->nodetag = 1;
    } 
    ;
Args:Exp COMMA Args { $$=newnode("Args",3,$1,$2,$3); }
    |Exp { $$=newnode("Args",1,$1); rpnum+=1; }
    ;
%%
