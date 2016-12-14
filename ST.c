#include "ST.h"

// 语法树建立，这里是二叉树，左孩子右兄弟表示法
struct astnode *
newnode(char*  gramname,int num,...) 
{
    va_list valist; // 定义变长参数列表
    struct astnode *a = (struct astnode*)malloc(sizeof(struct astnode));
    struct astnode *temp = (struct astnode*)malloc(sizeof(struct astnode));
    if(!a)
    {
        yyerror("out of space");
        exit(0);
    }
    a->gramname = gramname; // 语法单元名字
    va_start(valist,num); // 初始化变长参数为num后的参数

    if(num>0) // num>0表示非终结符
    {
        temp = va_arg(valist, struct astnode*); // 取变长参数列表中的第一个结点，设为a的左孩子
        a->l = temp;
        a->nodeline = temp->nodeline; // 传递行号
        //if(num == 1) // 对于右边只有一个符号的产生式，直接向左传递语义值、nodetag值
        //{
            a->content[0] = temp->content[0]; 
            a->nodetag = temp->nodetag;
        //}
        if(num != 1) // 取变长参数列表中的剩余结点，依次设置成右指针，为兄弟结点
        {
            for(int i=0; i<num-1; ++i)
            {
                temp->r = va_arg(valist,struct astnode*);
                temp = temp->r;
            }
        }
        printf("newnode:%s,%s\n", a->gramname, a->content[0]);
    }
    else // num==0表示空的语法单元/终结符
    {
        int t = va_arg(valist, int); // 取第1个变长参数，为所在行号
        a->nodeline = t;
        // 给终结符赋语义值
        if(!strcmp(a->gramname,"INTEGER")){
            a->value = atoi(yytext);
            //a->type = "int";
        }
        else if(!strcmp(a->gramname,"FLOAT")){
            a->value = atof(yytext);
            //a->type = "int";
        }
        else if(!strcmp(a->gramname,"OCT"))
        {
            char * p = yytext;
            int length = strlen(p);
            int n=0, k=1;
            for (int i = length-1; i > 0; i--)
            {
                n += k*(p[i]-48);
                k *= 8;
            }
            a->value = n;
            //a->type = "int";
        }
        else if(!strcmp(a->gramname,"HEX"))
        {
            char * p = yytext;
            int length = strlen(p);
            int n=0, k=1;
            for (int i = length-1; i > 1; i--)
            {
                if(p[i]>=48 && p[i]<=57) // 0-9
                    n += k*(p[i]-48);
                if(p[i]>=65 && p[i]<=90) //A-Z
                    n += k*(p[i]-55);
                if(p[i]>=97 && p[i]<=122) { // a-z
                    n += (p[i]-87)*k;
                }   
                k *= 16;
            }
            a->value = n;
            //a->type = "int";
        }
        else {
            if(!strcmp(a->gramname,"LC"))
                ++CurrentLevel;
            else if(!strcmp(a->gramname,"RC"))
                quitblock();
            a->content[0] = yytext;
        }
    }
    return a;
}

// 先序遍历语法树
void 
eval(struct astnode *a,int level) 
{
    if(a!=NULL)
    {
        for(int i=0; i<level; ++i) // 子结点相对父节点缩进2个空格
            printf("  ");
        if(a->nodeline != -1)  // 产生空的语法单元不需要打印信息
        {
            printf("%s ",a->gramname); // 打印语法单元名字，ID名字及类型、常数值
            if(!strcmp(a->gramname,"ID"))
                printf(":%s %s ",a->s->type,a->s->name);
            else if(!strcmp(a->gramname,"INTEGER")||!strcmp(a->gramname,"HEX")||!strcmp(a->gramname,"OCT")||!strcmp(a->gramname,"FLOAT")) 
                printf(":%f",a->value);
            else
                printf("(%d)",a->nodeline);
        }
        printf("\n");
        eval(a->l,level+1); //遍历左子树
        eval(a->r,level); //遍历右子树
    }
}

void clearast(struct astnode* current) {
    if(current != NULL)
    {
        struct astnode* l = current->l;
        struct astnode* r = current->r;
        free(current);
        clearast(l); 
        clearast(r); 
    }
}

// 建立符号表
void newsymbol(struct astnode* type, struct astnode* name) {

    struct symbol *p=(struct symbol*)malloc(sizeof(struct symbol));

    if(type->nodetag ==4) 
        p->tag = type->nodetag;
    else 
        p->tag = name->nodetag;
    p->line = name->nodeline;
    p->type = type->content[0];
    p->level = CurrentLevel;
    p->name = name->content[0];

    switch(p->tag) {
        case 1:
        p->value = name->value;
        if(havedefined(name)) {
            printf("Error type 3 at Line %d:Redefined Variable '%s'\n",yylineno,name->content[0]);
            return;
        }
        case 4:
        if(havedefined(name)) {
            printf("Error type 16 at Line %d:Duplicated structure name '%s'\n",yylineno,name->content[0]);
            return;
        }
        case 6:
        if(havedefined(name)) {
            printf("Error type 4 at Line %d:Redefined Function '%s'\n",yylineno,name->content[0]);
            return;
        }
        p->pnum = name->value;
    }    
    // 插在表头
    p->next = tablehead->next;
    tablehead->next = p;
    name->s = p;

    printf("newsymbol: %s\n", tablehead->next->name);
}

// 检测当前块内是否已定义同名变量
bool havedefined(struct astnode* current) {
    struct symbol* p = tablehead->next;
    while(p!=NULL && p->level==CurrentLevel)
    {
        if(!strcmp(p->name,current->content[0]))
            return false;
        p=p->next;
    }
    return true;
}

// 内->外 逐层检查符号表  检查符号是否能用 返回其tag值
int 
getdefined(struct astnode* current) {
    struct symbol* p = tablehead->next;
    while(p!=NULL)
    {
        if(!strcmp(p->name,current->content[0])) 
            current->s = p;
            return p->tag;
        p=p->next;
    }
    return 0;
}

// 释放当前level的变量
void 
quitblock() {
    --CurrentLevel;
    struct symbol* p = tablehead->next, * tp = p;
    while(p->level != CurrentLevel) {
        tp = p->next;
        free(p);
        p = tp;
    }
    tablehead->next = p;
}

void cleartable() {
    struct symbol* p = tablehead->next,  * tp = p;
    while(p->level != CurrentLevel) {
        tp = p->next;
        free(p);
        p = tp;
    }
    free(tablehead);
}

void checkreturn(struct astnode* exp) {
    struct symbol* p = tablehead->next;

    while(p!=NULL && p->tag==5)
        p=p->next;
    if(( !strcmp(exp->l->gramname,"INTEGER") && !strcmp(p->type,"int") ) || ( !strcmp(exp->l->gramname,"FLOAT") && !strcmp(p->type,"float")))
        return ;
    if(exp->nodetag!=0 && strcmp(p->type,exp->s->type)==0)
        return ;
    printf("Error type 8 at Line %d:Type mismatched for return.\n",yylineno);
}

void 
yyerror(char*s,...) //变长参数错误处理函数
{
    va_list ap;
    va_start(ap,s);
    fprintf(stderr,"%d:error:",yylineno);//错误行号
    vfprintf(stderr,s,ap);
    fprintf(stderr,"\n");
}


int main()
{
    tablehead=(struct symbol*)malloc(sizeof(struct symbol));//变量符号表头指针
    tablehead->next = NULL;
    rpnum = 0;
    CurrentLevel = 0;
    return yyparse(); //启动文法分析，调用词法分析
}