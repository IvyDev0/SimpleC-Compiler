#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#define MAXLIST 10
#define MAXNUM 1000
#define MAXDIM 3


extern int yylineno;
extern char* yytext;
extern int yyparse();
extern int yylex();
extern void yyerror(char* s,...);

// TODO: tag _ int、float

/* syntax tree node */
struct astnode
{
    int nodeline; // 行号
    int nodetag; // 0为纯右值/常数, 2为一般变量，3为数组，4为结构体，5为结构体对象，6为函数
    //char* type; // int / float
    char* content[MAXLIST]; // 可以表示xxxList的语义值
    char* gramname; // 语法单元名
    float value; // 用于传递形参个数/变量的值
    struct astnode *l; 
    struct astnode *r; 
    struct symbol *s;
};
struct astnode * newnode(char* gramname, int num, ...);
void eval(struct astnode *a,int level);
void clearast(struct astnode* current);

/* symbol table list */
struct symbol
{
    int line; // 行号
    int tag; // 1为变量，2为数组，3为结构体，4为结构体对象，5为函数
    int level; // 所在可见域
    char* name; // 变量/数组/结构体/函数名
    char* type; // int/float
    struct symbol *next; 

    float value; // 常数值(int/float的数据值)
    union { 
        char* varinfo[2]; // 存储结构体中定义的变量类型、变量名
        int dimension[MAXDIM][MAXNUM]; // 表示数组的维数、每维大小
    } var[MAXNUM]; 
    int currentvar; // 存内情变量var[MAXNUM]时计数
    int pnum; // 函数形参数个数
    char* rtype; // 函数实际返回值类型
}*tablehead; // 头指针的结构体里仅有一个指针。
bool havedefined(struct astnode* current); // 检测当前块内是否能定义（块内无同名变量）
void newsymbol(struct astnode* type, struct astnode* name);
void cleartable();
//void storeinner(int num,struct astnode* deflist); // num - 0:结构体，1:数组
int getdefined(struct astnode* current); // 返回tag值，若无定义则返回0
//bool getpnum(struct astnode* current);
int rpnum; // 实参个数
int CurrentLevel; // 记录当前层。遇‘{’加1，遇‘}’减1.
void quitblock(); // 用于删除块内定义的变量
void checkreturn(struct astnode* current);