#ifndef InterCodes
#define InterCodes
#include "ast.h"

struct arglist
{
	char* name;
	struct arglist* next;
};

// 生成目标代码添加：
FILE *fp; // 文件流指针
void openfile(); // 打开文件并初始化
void closefile(); // 关闭文件
char* treg[10]; // 记录寄存器存的变量名, t0 - t9 一共10个寄存器
int ensure(char* name); // 分配寄存器

char* combine_strings(char *a, char *b);
char* new_temp();
char* new_label();
void trans_vardecArray(struct astnode* vardec);
void trans_exp(struct astnode* exp, char* place);
void trans_args(struct astnode* args, struct arglist* arg_head);
void trans_cond(struct astnode* exp, char* labeltrue, char* labelfalse);
void trans_stmt(struct astnode* stmt);
void trans(struct astnode *current);

extern int tempcount, labelcount, regcount;
extern int firstreg; // 记录存一个变量最久的寄存器

extern struct para* addparalist(struct astnode* varlist);

#endif
