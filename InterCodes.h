#ifndef InterCodes
#define InterCodes 
#include "ast.h"

struct arglist
{
	char* name;
	struct arglist* next;
};

char* combine_strings(char *a, char *b);
char* new_temp();
char* new_label();
void trans_vardecArray(struct astnode* vardec);
void trans_exp(struct astnode* exp, char* place);
void trans_args(struct astnode* args, struct arglist* arg_head);
void trans_cond(struct astnode* exp, char* labeltrue, char* labelfalse);
void trans_stmt(struct astnode* stmt);
void trans(struct astnode *current);

extern int tempcount, labelcount;
extern struct para* addparalist(struct astnode* varlist);

#endif