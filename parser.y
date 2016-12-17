%{
#include "ast.h"
%}
%union {
struct astnode* a;
}
/*declare tokens*/
%token  <a> INTEGER FLOAT
%token <a> TYPE STRUCT RETURN IF ELSE WHILE ID SPACE SEMI COMMA ASSIGNOP RELOP
PLUS MINUS STAR DIV AND OR DOT NOT LP RP LB RB LC RC AERROR
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
        // 除了能识别实验要求中的16个语义错误，另外实现的还有：
        // 1-变量区分了作用域，2-一句话可声明多个变量（如 int i,j,k;）
        // 3-存储了结构体域内成员/函数形参，实现访问结构体成员的语义分析
        // 4-存储了数组的维度

        $$=newnode("Program",1,$1);
        printf("Print syntaxtree:\n");
        eval($$,0);
        freeast($$);
        freetable();
        printf("Finish printing.\n\n");
    }
    ;
ExtDefList:ExtDef ExtDefList { $$=newnode("ExtDefList",2,$1,$2); }
    | {$$=newnode("ExtDefList",0,-1);}
    ;
ExtDef:Specifier ExtDecList SEMI {
        $$=newnode("ExtDef",3,$1,$2,$3);
        $2->decnamelist = adddeclist($2);

        if($1->nodetag == 4) {
            $2->nodetag = 3;
            struct names *p = $2->decnamelist;
            while(p) {
                p->tag = 3;
                p = p->next;
            }
        }
        newsymbol($1,$2); // 构造符号表时：$1->content.c提供type；$2->arglist提供函数形参/结构体域成员；$2提供name,tag,line。
    }
    |Specifier SEMI { $$=newnode("ExtDef",2,$1,$2); }
    |Specifier FunDec Compst {
        $$=newnode("ExtDef",3,$1,$2,$3);
        $2->nodetag = 5; 
        newsymbol($1,$2); 
    }
    ;
ExtDecList:VarDec { $$=newnode("ExtDecList",1,$1); }
    |VarDec COMMA ExtDecList {$$=newnode("ExtDecList",3,$1,$2,$3);}
    ;
/*Specifier*/
Specifier:TYPE { $$=newnode("Specifier",1,$1); }
    |StructSpecifier { $$=newnode("Specifier",1,$1); }
    ;
StructSpecifier:STRUCT OptTag LC DefList RC { // 结构体定义
        $$=newnode("StructSpecifier",5,$1,$2,$3,$4,$5);
        $$->content = $2->content;
        $$->nodetag = 4;
        $1->nodetag = 4;
        $2->nodetag = 4;
        newsymbol($1,$2);
    }
    |STRUCT Tag { // 结构体引用
        $$=newnode("StructSpecifier",2,$1,$2);
        $$->content = $2->content;
        $$->nodetag = 4;
        if(getdefined($2) != 4) 
            printf("Error type 17 at Line %d: Undefined Structure '%s'\n",yylineno,$2->content.c);
    }
    ;
OptTag:ID { $$=newnode("OptTag",1,$1); }
    |{$$=newnode("OptTag",0,-1);}
    ;
Tag:ID {$$=newnode("Tag",1,$1);}
;
/*Declarators*/
VarDec:ID { $$=newnode("VarDec",1,$1); $$->nodetag = 1; }
    | VarDec LB INTEGER RB {$$=newnode("VarDec",4,$1,$2,$3,$4); $$->nodetag = 2; dim[dimcount] = (int)$3->content.f; ++dimcount; }
    ;
FunDec:ID LP VarList RP { $$=newnode("FunDec",4,$1,$2,$3,$4); }
    |ID LP RP { $$=newnode("FunDec",3,$1,$2,$3); $$->nodetag = 5; }
    ;
VarList:ParamDec COMMA VarList {$$=newnode("VarList",3,$1,$2,$3); ++paranum; }
    |ParamDec {$$=newnode("VarList",1,$1); paranum = 1; }
    ;
ParamDec:Specifier VarDec { 
        $$=newnode("ParamDec",2,$1,$2); 
        $$->type = $1->type; 
        $$->nodetag = $2->nodetag; 
        $$->content = $2->content;
    }
    ;
/*Statement*/
Compst:LC DefList StmtList RC { $$=newnode("Compst",4,$1,$2,$3,$4); }
    ;
StmtList:Stmt StmtList{$$=newnode("StmtList",2,$1,$2);}
    | {$$=newnode("StmtList",0,-1);}
    ;
Stmt:Exp SEMI {$$=newnode("Stmt",2,$1,$2);}
    |Compst {$$=newnode("Stmt",1,$1);}
    |RETURN Exp SEMI { 
        $$=newnode("Stmt",3,$1,$2,$3); 
        checkreturn($2);
    }
    |IF LP Exp RP Stmt {$$=newnode("Stmt",5,$1,$2,$3,$4,$5);}
    |IF LP Exp RP Stmt ELSE Stmt {$$=newnode("Stmt",7,$1,$2,$3,$4,$5,$6,$7);}
    |WHILE LP Exp RP Stmt {$$=newnode("Stmt",5,$1,$2,$3,$4,$5);}
    ;
/*Local Definitions*/
DefList:Def DefList{ $$=newnode("DefList",2,$1,$2); }
    | { $$=newnode("DefList",0,-1); }
    ;
Def:Specifier DecList SEMI {
        $$=newnode("Def",3,$1,$2,$3);
        $$->type = $1->content.c;
        $2->decnamelist = adddeclist($2);
        $$->decnamelist = $2->decnamelist;
        if($1->nodetag == 4) {
            $2->nodetag = 3;
            struct names *p = $2->decnamelist;
            while(p) {
                p->tag = 3;
                p = p->next;
            }
        }
        newsymbol($1,$2);
    }
    ;
DecList:Dec { $$=newnode("DecList",1,$1); }
    |Dec COMMA DecList { $$=newnode("DecList",3,$1,$2,$3); }
    ;
Dec:VarDec { $$=newnode("Dec",1,$1); }
    |VarDec ASSIGNOP Exp { $$=newnode("Dec",3,$1,$2,$3); }
    ;
/*Expressions*/
Exp:Exp ASSIGNOP Exp {
            $$=newnode("Exp",3,$1,$2,$3);
            $$->nodetag = 0;
            if($1->nodetag == 0) // Error type 6 检查赋值号左边是否出现右值
                printf("Error type 6 at Line %d: Pure Right Value At Left\n ",yylineno);
            else if(strcmp($1->type, $3->type)) // Error type 5 检查等号左右类型匹配判断
                printf("Error type 5 at Line %d: Type Mismatched for Assignment\n ",yylineno);
            else $1 = $2;
        }
        |Exp AND Exp{
            $$=newnode("Exp",3,$1,$2,$3);
            if(strcmp($1->type, $3->type)) // Error type 7 操作数类型不匹配
                printf("Error type 7 at Line %d: Type Mismatched for Operation\n ",yylineno);
            $$->nodetag = 0;
        }
        |Exp OR Exp{
            $$=newnode("Exp",3,$1,$2,$3);
            if(strcmp($1->type, $3->type)) // Error type 7 操作数类型不匹配
                printf("Error type 7 at Line %d: Type Mismatched for Operation\n ",yylineno);
            $$->nodetag = 0;
        }
        |Exp RELOP Exp{
            $$=newnode("Exp",3,$1,$2,$3);
            if(strcmp($1->type, $3->type)) // Error type 7 操作数类型不匹配
                printf("Error type 7 at Line %d: Type Mismatched for Operation\n ",yylineno);
            $$->nodetag = 0;
        }
        |Exp PLUS Exp{
            $$=newnode("Exp",3,$1,$2,$3);
            if(strcmp($1->type, $3->type)) // Error type 7 操作数类型不匹配
                printf("Error type 7 at Line %d: Type Mismatched for Operation\n ",yylineno);
            $$->nodetag = 0;
        }
        |Exp MINUS Exp {
            $$=newnode("Exp",3,$1,$2,$3);
            if(strcmp($1->type, $3->type)) // Error type 7 操作数类型不匹配
                printf("Error type 7 at Line %d: Type Mismatched for Operation\n ",yylineno);
            $$->nodetag = 0;
        }
        |Exp STAR Exp{
            $$=newnode("Exp",3,$1,$2,$3);
            if(strcmp($1->type, $3->type)) // Error type 7 操作数类型不匹配
                printf("Error type 7 at Line %d: Type Mismatched for Operation\n ",yylineno);
            $$->nodetag = 0;
        }
        |Exp DIV Exp{
            $$=newnode("Exp",3,$1,$2,$3);
            if(strcmp($1->type, $3->type)) // Error type 7 操作数类型不匹配
                printf("Error type 7 at Line %d: Type Mismatched for Operation\n ",yylineno);
            $$->nodetag = 0;
        }
        |LP Exp RP{ $$=newnode("Exp",3,$1,$2,$3); }
        |MINUS Exp { $$=newnode("Exp",2,$1,$2); }
        |NOT Exp { $$=newnode("Exp",2,$1,$2); }
        |ID LP Args RP { 
            $$=newnode("Exp",4,$1,$2,$3,$4);
            if(getdefined($1) != 5) // Error type 2 检查函数是否未定义就调用 
                printf("Error type 2 at Line %d: Undefined Function %s\n ",yylineno,$1->content.c);
            else if(getpnum($2) != rpnum) // 9 - 函数调用时实参与形参的数目不匹配 
                printf("Error type 9 at Line %d: Number of Parameters Mismatched\n ",yylineno);
            rpnum = 0;
        }
        |ID LP RP {
            $$=newnode("Exp",3,$1,$2,$3);
            if(getdefined($1) == 1) // 11- 对普通变量使用了函数调用的操作符 
                printf("Error type 11 at Line %d: Illegal call %s()\n ",yylineno,$1->content.c);
            else if(getpnum($2) != 0) 
                printf("Error type 9 at Line %d: Number of Parameters Mismatched\n ",yylineno);
        }
        |Exp LB Exp RB {
            $$=newnode("Exp",4,$1,$2,$3,$4);
            if(getdefined($1) == 0)
                printf("Error type 1 at Line %d: Undefined Variable '%s'\n ",yylineno,$1->content.c);
            else if(getdefined($1) != 2) // Error type 10 对非数组类型使用了[]数组访问的操作符 
                printf("Error type 10 at Line %d: '%s'is not an array.\n ",yylineno,$1->content.c);
            else if(!strcmp($3->type,"int")) // Error type 12 数组访问下标出现了非整数 
                printf("Error type 12 at Line %d: %.1f is not a integer.\n",yylineno,$3->content.f);
            else 
                $$->nodetag = 2;
        }
        |Exp DOT ID { 
            $$=newnode("Exp",3,$1,$2,$3);
            if(getdefined($1) != 3) // Error type 13 对非结构体变量使用了.操作
                printf("Error type 13 at Line %d:Illegal use of '.'.\n",yylineno);
            else { // Error type 14 检查是否访问结构体中未定义的域
                visitstruct($1,$3);
            }
        }
        |ID { 
            $$=newnode("Exp",1,$1); 
            if(getdefined($1) == 0)
                printf("Error type 1 at Line %d: Undifined Variable %s\n", yylineno, $1->content.c);
        }
        |INTEGER { $$=newnode("Exp",1,$1); $$->nodetag = 0; }
        |FLOAT { $$=newnode("Exp",1,$1); $$->nodetag = 0; }
        ;
Args:Exp COMMA Args {$$=newnode("Args",3,$1,$2,$3);}
        |Exp { $$=newnode("Args",1,$1); ++rpnum; }
        ;
%%