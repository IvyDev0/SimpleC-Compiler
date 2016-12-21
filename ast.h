#include <string.h> 
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>

#define MAXNUM 100

extern int yylineno;
extern char* yytext;
extern int yyparse();
extern int yylex();
extern void yyerror(char*s,...);


struct names // 表示同一类型的变量，如 int i,j,k[9];
{
	char* name;
	int tag; // 1-普通变量，2-数组变量，4-结构体对象
	struct names* next;
};

struct para // 记录结构体域内成员/函数形参
{
    char* type;
    struct names* namelist;
    int tag; // 1-普通变量，2-数组变量，4-结构体对象
    struct para* next;
};

// 语法树
struct astnode
{
    int nodeline; // 行号
    char* gramname; // 语法单元的名字
    int nodetag; // 0为常数/纯右值，1为普通变量，2为数组，3为结构体对象，4为结构体，5为函数
    union {
    	char* c;
    	float f;
    } content; // 语法单元语义值
    char* type; // int/float: 用于类型匹配判断

    struct names* decnamelist; // 记录DecList, ExtDecList中的多个同类的变量
    struct astnode* l;
    struct astnode* r;

    // 为了中间代码生成增加的属性
    //char* tpname;
    //char* place;

};
struct names *adddeclist(struct astnode* declist);

struct astnode *newnode(char* name,int num,...);
void eval(struct astnode*,int level);
void freeast(struct astnode* current);
int paranum; // 记录函数形参个数
int rpnum; // 记录函数实参个数
int CurrentLevel; // 记录当前作用域
char* rtype; // 记录当前函数实际返回值类型
int dim[MAXNUM];
int dimcount;

int tempcount, labelcount; 


// 符号表
struct symbol
{
    int line; // 行号
    int tag; // 1为普通变量，2为数组，3为结构体对象，4为结构体，5为函数
    int level; // 所在可见域
    char* name; // 符号名
    char* type; // int/float
    struct symbol* next; 

    union { 
		struct para* paralist; // 记录结构体域内成员/函数形参        
		int dimension[MAXNUM]; // 记录数组的每维大小
    } info;
    int pnum; // 函数形参数个数

}*tablehead, *currentfunction; // 头指针的结构体里仅有一个next指针。
struct para* addparalist(struct astnode* varlist);
void addfunction(struct astnode* funid);

bool havedefined(char* name); // 检测当前块内是否能定义（块内无同名变量）
void newsymbol(struct astnode* type, struct astnode* name);
void freetable();

int getdefined(struct astnode* current); // 返回tag值，若无定义则返回0
void visitstruct(struct astnode* exp, struct astnode* id);

int getpnum(struct astnode* current);
int paramark;
int rpnum; // 记录实参个数
int CurrentLevel; // 记录当前层。遇‘{’加1，遇‘}’减1.
void quitblock(); // 用于删除块内定义的变量
void checkreturn(struct astnode* current);
