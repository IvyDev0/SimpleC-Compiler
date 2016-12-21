#include "InterCodes.h"

char* combine_strings(char *a, char *b)
{
    char *ptr;
    int lena=(int)strlen(a), lenb=(int)strlen(b);
    int i,l=0;
    ptr = (char *)malloc((lena+lenb+1) * sizeof(char));
    for(i=0;i<lena;i++)
        ptr[l++]=a[i];
    for(i=0;i<lenb;i++)
        ptr[l++]=b[i];
    ptr[l]='\0';
    return(ptr);
}
// 生成中间变量
char* new_temp()
{
	++tempcount;
    char c[20], *tmp;
    sprintf(c, "%d", tempcount);
    tmp = combine_strings("t",c);
    return tmp;
}
char* new_label()
{
	++labelcount;
    char c[20], *label;
    sprintf(c, "%d", labelcount);
    label = combine_strings("label",c);
    return label;
}

void trans_exp(struct astnode* exp, char* place)
{
	if(!strcmp(exp->l->gramname,"INTEGER") || !strcmp(exp->l->gramname,"FLOAT"))
		printf("%s := #%0.0f\n", place, exp->l->content.f); 
	else if(!strcmp(exp->l->gramname,"ID") && exp->l->r==NULL) 
		printf("%s := %s\n", place, exp->l->content.c); 
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"ASSIGNOP") && !strcmp(exp->l->l->gramname,"ID"))
	{
		char* varname = exp->l->l->content.c;
		char* tmp = new_temp();
		trans_exp(exp->l->r->r, tmp);
		printf("%s := %s\n", varname, tmp);
	}
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"PLUS"))
	{
		char* tmp1 = new_temp();
		char* tmp2 = new_temp();
		trans_exp(exp->l, tmp1);
		trans_exp(exp->l->r->r, tmp2);
		printf("%s := %s + %s\n", place, tmp1, tmp2);
	}
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"STAR"))
	{
		char* tmp1 = new_temp();
		char* tmp2 = new_temp();
		trans_exp(exp->l, tmp1);
		trans_exp(exp->l->r->r, tmp2);
		printf("%s := %s * %s\n", place, tmp1, tmp2);
	}
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"DIV"))
	{
		char* tmp1 = new_temp();
		char* tmp2 = new_temp();
		trans_exp(exp->l, tmp1);
		trans_exp(exp->l->r->r, tmp2);
		printf("%s := %s / %s\n", place, tmp1, tmp2);
	}
	else if(!strcmp(exp->l->gramname,"MINUS"))
	{
		char* tmp = new_temp();
		trans_exp(exp->l->r, tmp);
		printf("%s := #0 - %s\n", place, tmp);
	}
	else if(  !strcmp(exp->l->gramname,"NOT")  ||   ((exp->l->r) && ( !strcmp(exp->l->r->gramname,"AND")||!strcmp(exp->l->r->gramname,"OR")||!strcmp(exp->l->r->gramname,"RELOP") )) )
	{
		char* label1 = new_label();
		char* label2 = new_label();
		printf("%s := #0\n", place);
		trans_cond(exp, label1, label2);
		printf("LABEL %s:\n%s := #1\nLABEL %s:\n", label1, place, label2);
	}
	
	if(exp->l->r && !strcmp(exp->l->r->gramname,"LP"))
	{

		if(!strcmp(exp->l->r->r->gramname,"Args"))
		{

			char* function = exp->l->content.c;
    		struct arglist* arg_head = (struct arglist*)malloc(sizeof(struct arglist));
    		arg_head->next = NULL;

			if(!strcmp(function,"write"))
			{
				trans_args(exp->l->r->r, arg_head);
				printf("WRITE %s\n", arg_head->next->name);
			}
			else
			{
				trans_args(exp->l->r->r, arg_head);
				struct arglist* p = arg_head->next;
				while(p)
				{
					printf("ARG %s\n", p->name);
					p = p->next;
				}
				printf("%s := CALL %s\n", place, function);
			}
		}
		else
		{
			char* function = exp->l->content.c;
			if(!strcmp(function,"read"))
				printf("READ %s\n", place);
			else
				printf("%s := CALL %s\n", place, function);
		}
	}
}
void trans_args(struct astnode* args, struct arglist* arg_head)
{
	if(args->l->r && !strcmp(args->l->r->gramname,"COMMA")) // Args -> Exp COMMA Args1
	{
		char* tmp = new_temp();
		trans_exp(args->l, tmp);
    	struct arglist* newarg = (struct arglist*)malloc(sizeof(struct arglist));
    	newarg->name = tmp;
    	newarg->next = arg_head->next;
    	arg_head->next = newarg;
    	trans_args(args->l->r->r, arg_head);
	}
	else // Args -> Exp
	{
		char* tmp = new_temp();
		trans_exp(args->l, tmp);
		struct arglist* newarg = (struct arglist*)malloc(sizeof(struct arglist));
    	newarg->name = tmp;
    	newarg->next = arg_head->next;
    	arg_head->next = newarg;
	}
}
void trans_cond(struct astnode* exp, char* labeltrue, char* labelfalse)
{
	if(!strcmp(exp->l->r->gramname,"RELOP"))
	{
		char* tmp1 = new_temp();
		char* tmp2 = new_temp();
		trans_exp(exp->l, tmp1);
		trans_exp(exp->l->r->r, tmp2);
		char* op = exp->l->r->content.c;
		printf("IF %s %s %s GOTO %s\nGOTO %s\n", tmp1, op, tmp2, labeltrue, labelfalse);
	}
	else if(!strcmp(exp->l->gramname,"NOT"))
	{
		trans_cond(exp, labelfalse, labeltrue);
	}
	else if(!strcmp(exp->l->r->gramname,"AND"))
	{
		char* label = new_label();
		trans_cond(exp->l, label, labelfalse);
		printf("LABEL %s:\n", label);
		trans_cond(exp->l->r->r, labeltrue, labelfalse);
	}
	else if(!strcmp(exp->l->r->gramname,"OR"))
	{
		char* label = new_label();
		trans_cond(exp->l->r->r, labeltrue, label);
		printf("LABEL %s:\n", label);
		trans_cond(exp->l, labeltrue, labelfalse);
	}
	else
	{
		char* tmp = new_temp();
		trans_exp(exp, tmp);
		printf("IF %s != #0 GOTO %s\nGOTO %s\n", tmp, labeltrue, labelfalse);
	}
}
void trans_stmt(struct astnode* stmt)
{
	if(!strcmp(stmt->l->gramname,"Exp"))
		trans_exp(stmt->l, NULL);
	else if(!strcmp(stmt->l->gramname,"CompSt"))
		trans(stmt->l);
	else if(!strcmp(stmt->l->gramname,"RETURN"))
	{
		char* tmp = new_temp();
		trans_exp(stmt->l->r, tmp);
		printf("RETURN %s\n", tmp);
	}
	else if(!strcmp(stmt->l->gramname,"IF"))
	{
		if(stmt->l->r->r->r->r->r)
		{
			char* label1 = new_label();
			char* label2 = new_label();
			char* label3 = new_label();
			trans_cond(stmt->l->r->r, label1, label2);
			printf("LABEL %s:\n", label1);
			trans_stmt(stmt->l->r->r->r->r);
			printf("GOTO %s\nLABEL %s:\n", label3, label2);
			trans_stmt(stmt->l->r->r->r->r->r->r);
			printf("LABEL %s:\n", label3);
		}
		else
		{
			char* label1 = new_label();
			char* label2 = new_label();
			trans_cond(stmt->l->r->r, label1, label2);
			printf("LABEL %s:\n", label1);
			trans_stmt(stmt->l->r->r->r->r);
			printf("LABEL %s:\n", label2);
		}
	}
	else if(!strcmp(stmt->l->gramname,"WHILE"))
	{
		char* label1 = new_label();
		char* label2 = new_label();
		char* label3 = new_label();
		printf("LABEL %s:\n", label1);
		trans_cond(stmt->l->r->r, label2, label3);
		printf("LABEL %s:\n", label2);
		trans_stmt(stmt->l->r->r->r->r);
		printf("GOTO %s\nLABEL %s:\n", label1, label3);
	}
}

void trans(struct astnode *current)
{
	if(!strcmp(current->gramname,"FunDec")) 
    {
		printf("FUNCTION %s:\n",current->l->content.c);
		if(!strcmp(current->l->r->r->gramname,"VarList")) 
		{
			struct para* paralist = addparalist(current->l->r->r);
			if(paralist)
				printf("PARAM ");
			while(paralist)
			{
				printf("%s ", paralist->namelist->name);
				paralist = paralist->next;
			}	
			printf("\n");
		}
		if(current->l)
			trans(current->l);
		if(current->r)
			trans(current->r);
	}
	else if(!strcmp(current->gramname,"Stmt"))
	{
		trans_stmt(current);
		if(current->l)
			trans(current->l);
		if(current->r)
			trans(current->r);
	}
	else if(!strcmp(current->gramname,"Exp"))
	{
		char* tmp = new_temp();
		trans_exp(current, tmp);
		if(current->l)
			trans(current->l);
		if(current->r)
			trans(current->r);
	}
	// Dec -> VarDec, 当VarDec是数组或结构体时， 
	else if(!strcmp(current->gramname,"Dec") && (current->l->r==NULL))
	{
		if(!strcmp(current->l->l->gramname,"VarDec"))
			trans_vardecArray(current->l->l);
	}
	// Dec -> VarDec ASSIGNOP Exp
	else if(!strcmp(current->gramname,"VarDec") && current->r && !strcmp(current->r->gramname,"ASSIGNOP"))
	{
		char* tmp1 = new_temp();
		trans_exp(current->r->r, tmp1);
		if(!strcmp(current->l->gramname,"ID"))
        	printf("%s := %s\n",current->l->content.c, tmp1);
        else // VarDec -> VarDec LB INTEGER RB
        	trans_vardecArray(current->l);       	
	}
	else
	{
		if(current->l)
			trans(current->l);
		if(current->r)
			trans(current->r);
	}
}
void trans_vardecArray(struct astnode* vardec)
{
	struct astnode* p = vardec;
	char* idname;
	float size = 4.0;

	while(p) {
		if(!strcmp(p->l->gramname,"ID")) {
			idname = p->l->content.c;
			break;
		}
		else
			size *= p->l->r->r->content.f;
		p = p->l;
	}
	
	printf("DEC %s %0.0f\n", idname, size);
}