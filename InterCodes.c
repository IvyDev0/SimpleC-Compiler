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
// Exp
void trans_exp(struct astnode* exp, char* place)
{
	if(!strcmp(exp->l->gramname,"INTEGER") || !strcmp(exp->l->gramname,"FLOAT"))
	{
        int reg = ensure(place);
        printf("%s := #%0.0f\n", place, exp->l->content.f);
        fprintf(fp,"  li $t%d, %0.0f\n", reg, exp->l->content.f);
    }
	else if(!strcmp(exp->l->gramname,"ID") && exp->l->r==NULL)
	{
        int reg = ensure(place);
        if (strcmp(place,exp->content.c) != 0) {
            int reg2 = ensure(exp->l->content.c);
            printf("%s := %s\n", place, exp->l->content.c);
            fprintf(fp,"  move $t%d, $t%d\n", reg, reg2);
        }
    }
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"ASSIGNOP") && !strcmp(exp->l->l->gramname,"ID"))
	{
		char* varname = exp->l->l->content.c;
		char* tmp = new_temp();
        trans_exp(exp->l->r->r, varname);
		printf("%s := %s\n", varname, tmp);
	}
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"PLUS"))
	{
		char* tmp1 = new_temp();
		char* tmp2 = new_temp();
        int reg = ensure(place);

        if(exp->l->nodetag==0 && exp->l->r->r->nodetag == 1) // 0是常数 1是普通变量
        {
        	// 加立即数的指令
            int reg2 = ensure(exp->l->r->r->content.c);
        	fprintf(fp,"  addi $t%d, %0.0f, $t%d \n", reg, exp->l->content.f, reg2);
        }
        else if(exp->l->nodetag==1 && exp->l->r->r->nodetag == 0)
        {
        	int reg1 = ensure(exp->l->content.c);
        	fprintf(fp,"  addi $t%d, $t%d, %0.0f\n", reg, reg1, exp->l->r->r->content.f);
        }
        else if(exp->l->nodetag==1 && exp->l->r->r->nodetag == 1)
        {
        	int reg1 = ensure(exp->l->content.c);
            int reg2 = ensure(exp->l->r->r->content.c);
            fprintf(fp,"  add $t%d, $t%d, $t%d \n", reg, reg1, reg2);
        }
        else
        {
        	trans_exp(exp->l, tmp1);
        	trans_exp(exp->l->r->r, tmp2);
        }
        //printf("%s := %s + %s\n", place, tmp1, tmp2);
	}
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"STAR"))
	{
		char* tmp1 = new_temp();
		char* tmp2 = new_temp();
        int reg = ensure(place);
        int reg1 = ensure(tmp1);
        int reg2 = ensure(tmp2);
		trans_exp(exp->l, tmp1);
		trans_exp(exp->l->r->r, tmp2);
		printf("%s := %s * %s\n", place, tmp1, tmp2);
        fprintf(fp,"  mul $t%d, $t%d, $t%d \n", reg, reg1, reg2);
	}
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"DIV"))
	{
		char* tmp1 = new_temp();
		char* tmp2 = new_temp();
        int reg = ensure(place);
        int reg1 = ensure(tmp1);
        int reg2 = ensure(tmp2);
		trans_exp(exp->l, tmp1);
		trans_exp(exp->l->r->r, tmp2);
		printf("%s := %s / %s\n", place, tmp1, tmp2);
        fprintf(fp,"  div $t%d, $t%d\n", reg1, reg2);
        fprintf(fp,"  mflo $t%d \n", reg);
	}
	else if(!strcmp(exp->l->gramname,"MINUS"))
	{
		char* tmp = new_temp();
        int reg2 = ensure(tmp);
		trans_exp(exp->l->r, tmp);
		printf("%s := #0 - %s\n", place, tmp);
        int reg1 = ensure("zero");
        int reg = ensure(place);
        fprintf(fp,"  li $t%d 0\n  sub $t%d,  $t%d, $t%d \n", reg1, reg1, reg2, reg);
	}
    // Exp -> Exp RELOP Exp
	else if(  !strcmp(exp->l->gramname,"NOT")  ||   ((exp->l->r) && ( !strcmp(exp->l->r->gramname,"AND")||!strcmp(exp->l->r->gramname,"OR")||!strcmp(exp->l->r->gramname,"RELOP") )) )
	{
		char* label1 = new_label();
		char* label2 = new_label();
		printf("%s := #0\n", place);
		trans_cond(exp, label1, label2);
		printf("LABEL %s:\n%s := #1\nLABEL %s:\n", label1, place, label2);
	}
    // 调用函数
	else if(exp->l->r && !strcmp(exp->l->r->gramname,"LP")) // ID LP Args RP
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
                int reg1 = ensure(arg_head->next->name);
                fprintf(fp,"  move $a0, $t%d\n  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n  jal write\n  lw $ra, 0($sp)\n  addi $sp, $sp, 4\n", reg1);
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
                int reg = ensure(place);
                fprintf(fp,"  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n  jal %s\n  lw $ra, 0($sp)\n  addi $sp, $sp, 4\n  move $t%d, $v0\n", function, reg);
			}
		}
		else // ID LP RP
		{
            fprintf(fp, "  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n");
			char* function = exp->l->content.c;
			if(!strcmp(function,"read"))
            {
				printf("READ %s\n", place);
                int reg = ensure(place);
                fprintf(fp,"  jal read\n  lw $ra, 0($sp)\n  addi $sp, $sp, 4\n  move $t%d, $v0\n", reg);
            }
			else
            {
				printf("%s := CALL %s\n", place, function);
                int reg = ensure(place);
                fprintf(fp,"  jal %s\n  lw $ra, 0($sp)\n  addi $sp, $sp, 4\n  move $t%d, $v0\n", function, reg);
            }
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
		struct arglist* newarg = (struct arglist*)malloc(sizeof(struct arglist));
    	newarg->name = args->l->content.c;
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
        int reg1 = ensure(exp->l->content.c);
        int reg2 = ensure(exp->l->r->r->content.c);
		//trans_exp(exp->l, tmp1);
		//trans_exp(exp->l->r->r, tmp2);
		char* op = exp->l->r->content.c;
		printf("IF %s %s %s GOTO %s\nGOTO %s\n", tmp1, op, tmp2, labeltrue, labelfalse);
        if(!strcmp(op, "=="))
            fprintf(fp, "  beq");
        else if (!strcmp(op, ">"))
            fprintf(fp, "  bgt");
        else if (!strcmp(op, ">="))
            fprintf(fp, "  bge");
        else if (!strcmp(op, "<"))
            fprintf(fp, "  blt");
        else if (!strcmp(op, "<="))
            fprintf(fp, "  ble");
        fprintf(fp, " $t%d, $t%d, %s\n  j %s\n", reg1, reg2, labeltrue, labelfalse);
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
        int reg = ensure(tmp);
        fprintf(fp, "  bne $%d, 0, %s \n  j %s\n", reg, labeltrue, labelfalse);
    }
}
void trans_stmt(struct astnode* stmt)
{
	if(!strcmp(stmt->l->gramname,"Exp"))
        trans_exp(stmt->l, stmt->l->content.c);
	else if(!strcmp(stmt->l->gramname,"Compst"))
		trans(stmt->l->l);
	else if(!strcmp(stmt->l->gramname,"RETURN"))
	{
		char* tmp = new_temp();
        int reg = ensure(tmp);
		trans_exp(stmt->l->r, tmp);
		printf("RETURN %s\n", tmp);
        fprintf(fp, "  move $v0, $t%d\n  jr $ra\n", reg);
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
            fprintf(fp, "%s: \n", label1);
			trans_stmt(stmt->l->r->r->r->r);
			printf("GOTO %s\nLABEL %s:\n", label3, label2);
            fprintf(fp, "  j %s \n%s: \n", label3, label2);
			trans_stmt(stmt->l->r->r->r->r->r->r);
			printf("LABEL %s:\n", label3);
            fprintf(fp, "%s: \n", label3);
		}
		else
		{
			char* label1 = new_label();
			char* label2 = new_label();
			trans_cond(stmt->l->r->r, label1, label2);
			printf("LABEL %s:\n", label1);
            fprintf(fp, "%s: \n", label1);
			trans_stmt(stmt->l->r->r->r->r);
			printf("LABEL %s:\n", label2);
            fprintf(fp, "%s: \n", label2);
		}
	}
	else if(!strcmp(stmt->l->gramname,"WHILE"))
	{
		char* label1 = new_label();
		char* label2 = new_label();
		char* label3 = new_label();
        printf("LABEL %s:\n", label1);
        fprintf(fp, "%s: \n", label1);
		trans_cond(stmt->l->r->r, label2, label3); // 条件Exp
		printf("LABEL %s:\n", label2);
        fprintf(fp, "%s: \n", label2);
		trans_stmt(stmt->l->r->r->r->r);
		printf("GOTO %s\nLABEL %s:\n", label1, label3);
        fprintf(fp, "  j %s \n%s: \n", label1, label3);
	}
}

void trans(struct astnode *current)
{
	if(!strcmp(current->gramname,"FunDec"))
    {
		printf("FUNCTION %s:\n",current->l->content.c);
        fprintf(fp,"%s: \n",current->l->content.c);
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
		if(current->r)
			trans(current->r);
	}
	else if(!strcmp(current->gramname,"Stmt"))
	{
		trans_stmt(current);
		if(current->r)
			trans(current->r);
	}
	else if(!strcmp(current->gramname,"Exp"))
	{
		char* tmp = new_temp();
		trans_exp(current, tmp);
	}
	// Dec -> VarDec
	else if(!strcmp(current->gramname,"Dec") && (current->l->r==NULL))
	{
		if(!strcmp(current->l->l->gramname,"VarDec")) // 当VarDec是数组或结构体时
			trans_vardecArray(current->l->l);
	}
	// Dec -> VarDec ASSIGNOP Exp
	else if(!strcmp(current->gramname,"VarDec") && current->r && !strcmp(current->r->gramname,"ASSIGNOP"))
	{
        char* tmp1 = new_temp();
		trans_exp(current->r->r, current->content.c);
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
// File OS:
void openfile()
{
    fp = fopen("./final.asm","w");
    char* p = ".data \n_prompt: .asciiz \"Enter an integer:\" \n_ret: .asciiz ";
    fprintf(fp,"%s",p);
    fprintf(fp,"%s","\"\\n\"  \n");
    p = ".globl main \n.text \n";
    fprintf(fp,"%s",p);
    fprintf(fp,"read:\n  li $v0, 4\n  la $a0, _prompt\n  syscall\n  li $v0, 5\n  syscall\n  jr $ra\n\n");
    fprintf(fp,"write:\n  li $v0, 1\n  syscall\n  li $v0, 4\n  la $a0, _ret\n  syscall\n  move $v0, $0\n  jr $ra\n\n");
}
void closefile()
{
    fclose(fp);
}
// Allocate Register
int ensure(char* name)
{
    int i, exist = 0;
    for (i = 0; i < regcount; ++i) {
        if(!strcmp(treg[i], name))
        {
            exist = 1;
            break;
        }
    }
    if(exist)
        return i;
    else
    {
        if(regcount == 10)
        {
            treg[firstreg] = name; // 将最先进入寄存器的变量换走
            return firstreg;
        }
        else
        {
            treg[regcount] = name;
            return regcount++;
        }
    }
}
