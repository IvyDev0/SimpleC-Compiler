#include "ast.h"

int i;
struct astnode *newnode(char*  gramname,int num,...)//抽象语法树建立
{
    va_list valist; //定义变长参数列表
    struct astnode *a=(struct astnode*)malloc(sizeof(struct astnode));//新生成的父节点
    struct astnode *temp=(struct astnode*)malloc(sizeof(struct astnode));
    if(!a)
    {
        yyerror("out of space");
        exit(0);
    }
    a->gramname = gramname;
    a->decnamelist = NULL;
    va_start(valist,num); // 初始化变长参数为num后的参数

    if(num>0) // num>0为非终结符，左孩子右兄弟表示法
    {
        temp=va_arg(valist, struct astnode*); // 取变长参数列表中的第一个结点设为a的左孩子

        a->l = temp;
        a->r = NULL;
        // 左结点直接向父节点传递的值：行号、语义、nodetag、type
        a->nodeline = temp->nodeline;
        a->content = temp->content;
        a->nodetag = temp->nodetag;
        a->type = temp->type;

        if(num > 1)  
        {
            for(i=0; i<num-1; ++i) // 依次取变长参数列表中的剩余结点，为左结点
            {
                temp->r = va_arg(valist,struct astnode*);
                temp = temp->r;
            }
        }
    }
    else // num==0为终结符或产生空的语法单元：第1个变长参数表示行号，产生空的语法单元行号为-1。
    {
        int t=va_arg(valist, int); //取第1个变长参数
        a->nodeline=t;
        if(!strcmp(a->gramname,"INTEGER")) 
        {
            a->type="int";
            a->content.f=atof(yytext);
        }
        else if(!strcmp(a->gramname,"FLOAT"))
        {
            a->type="float";
            a->content.f=atof(yytext);
        }
        else if(!strcmp(a->gramname,"OCT"))
        {
            a->type="int";

            char * p = yytext;
            int length = strlen(p);
            int n=0, k=1;
            
            for (int i = length-1; i > 0; i--)
            {
                n += k*(p[i]-48);
                k *= 8;
            }
            a->content.f = n;
        }
        else if(!strcmp(a->gramname,"HEX"))
        {
            a->type="int";

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
            a->content.f = n;
        }
        else
        {
            if(!strcmp(a->gramname,"LC"))
                ++CurrentLevel;
            else if(!strcmp(a->gramname,"RC"))
                quitblock();
            // 意为存储当前词法分析器返回的yytext字符串。若直接a->content.c=yytext，则a->content.c之后会变化。（由于是指针）
            char* s;
            s=(char*)malloc(sizeof(char* )*40);
            strcpy(s,yytext);     
            a->content.c = s;
        }
    }
    printf("newnode:  %d, %s\n", a->nodeline, a->gramname);
    return a;
}

void eval(struct astnode *a,int level) // 先序遍历语法树
{
    if(a!=NULL)
    {
        for(i=0; i<level; ++i) // 孩子结点相对父节点缩进
            printf("  ");
        if(a->nodeline!=-1)  // 产生空的语法单元不需要打印信息
        {
            printf("%s",a->gramname);
            if((!strcmp(a->gramname,"ID"))||(!strcmp(a->gramname,"TYPE")))
                printf(":%s ",a->content.c);
            else if(!strcmp(a->gramname,"INTEGER")||(!strcmp(a->gramname,"OCT"))||(!strcmp(a->gramname,"HEX")))
                printf(":%0.0f",a->content.f);
            else
                printf("(%d)",a->nodeline);
        }
        printf("\n");
        eval(a->l,level+1); // 遍历左子树
        eval(a->r,level); // 遍历右子树
    }
}

void freeast(struct astnode* current) {
    if(current != NULL)
    {
        struct astnode* l = current->l;
        struct astnode* r = current->r;
        free(current);
        freeast(l); 
        freeast(r); 
    }
}

// 对于 Dec COMMA DecList -> DecList,  Dec -> DecList，传递变量，加入DecList的decnamelist。
struct names *adddeclist(struct astnode* declist) {
    struct names *decnamelist = (struct names*)malloc(sizeof(struct names));
    struct astnode *a = declist;
    struct names *p = decnamelist;

    while(a->l->r) {
        p->name = a->l->content.c;
        p->tag = a->l->nodetag;

        a = a->l->r->r;
        p->next = (struct names*)malloc(sizeof(struct names));
        p = p->next;
    }

    p->name = a->l->content.c;
    p->tag = a->l->nodetag;
    p->next = NULL;

    p = decnamelist;
    while(p) 
        p = p->next;
    
    return decnamelist;        
}

struct para *adddeflist(struct astnode* deflist) { 
    struct para *paralist = (struct para*)malloc(sizeof(struct para));
    struct astnode *a = deflist;
    struct para *p = paralist;

    while(a->l->r->nodeline != -1) {
        p->type = a->l->type;
        p->namelist = a->l->decnamelist;

        a = a->l->r;
        p->next = (struct para*)malloc(sizeof(struct para));
        p = p->next;
    }
    p->type = a->type;
    p->namelist = a->l->decnamelist;
    p->next = NULL;
    

    p = paralist;
    while(p) {
        printf("------ deflist: %s, %s\n", p->type, p->namelist->name);
        p = p->next;
    }

    return paralist;
}


// VarList:ParamDec COMMA VarList | ParamDec  记录函数所有形参
struct para *addparalist(struct astnode* varlist) { 
    

    struct para *paralist = (struct para*)malloc(sizeof(struct para));
    struct astnode *a = varlist;
    struct para *p = paralist;

    while(a->l->r) {
        //printf("------------addparalist\n");

        p->type = a->l->type;
        p->namelist->name = a->l->content.c;
        p->namelist->next = NULL;

        a = a->l->r->r;
        p->next = (struct para*)malloc(sizeof(struct para));
        p = p->next;
    }
    //printf("------------addparalist\n");

    p->type = a->type;
    p->namelist = (struct names*)malloc(sizeof(struct names));
    p->namelist->name = a->content.c;
    p->namelist->next = NULL;
    p->next = NULL;

    return paralist;
}
void addfunction(struct astnode* funid)
{
    currentfunction->name = funid->content.c;
    currentfunction->pnum = paranum;
    printf("------addfunction: %s\n", currentfunction->name);
}

// 建立符号表
// $1->content.c提供type；$2->decnamelist提供同一type的name,tag；line。
// 全局变量CurrentLevel提供level；paranum提供形参个数pnum。
void newsymbol(struct astnode* type, struct astnode* arg) 
{

    struct symbol *p = (struct symbol*)malloc(sizeof(struct symbol));

    p->type = type->content.c; 
    p->line = arg->nodeline;
    p->tag = arg->nodetag;

    p->name = arg->content.c;
    p->level = CurrentLevel;

    if(arg->nodetag == 5) {
        if(!strcmp(arg->l->r->r->gramname,"RP")){
            p->info.paralist = NULL;
        }
        else
            p->info.paralist = addparalist(arg->l->r->r);
        if(havedefined(p->name)) 
            printf("Error type 4 at Line %d: Redefined Function '%s'\n",yylineno,p->name);
        p->pnum = paranum;
        paranum = 0;

        // 插在表头
        p->next = tablehead->next;
        tablehead->next = p;
        // 清空 currentfunction
        currentfunction->name = "";
        currentfunction->pnum = 0;

        printf("newsymbol: %s, tag:%d, type: %s, CurrentLevel: %d\n", tablehead->next->name, tablehead->next->tag, tablehead->next->type, CurrentLevel);

    } else if(arg->nodetag == 4) {

        if(arg->r->r->nodeline != -1)
            p->info.paralist = adddeflist(arg->r->r);
        else
            p->info.paralist = NULL;
        if(havedefined(p->name)) 
            printf("Error type 16 at Line %d: Duplicated Structure Name '%s'\n",yylineno,p->name);
        // 插在表头
        p->next = tablehead->next;
        tablehead->next = p;

        printf("newsymbol: %s, tag:%d, type: %s, CurrentLevel: %d\n", tablehead->next->name, tablehead->next->tag, tablehead->next->type, CurrentLevel);

    } else {
        struct names *namelist = arg->decnamelist;
        while(namelist) {
            struct symbol *p = (struct symbol*)malloc(sizeof(struct symbol));
            p->type = type->content.c; 
            p->line = arg->nodeline;
            if(paramark == 1)
                p->level = CurrentLevel+1;
            else
                p->level = CurrentLevel;            

            p->tag = namelist->tag;
            p->name = namelist->name;

            if(p->tag == 2) 
            {
                for (int i = 0; i < dimcount; ++i)
                {
                    p->info.dimension[i] = dim[i];
                    dim[i] = 0;
                }
                dimcount = 0;
            }

            if(havedefined(p->name)) 
                printf("Error type 3 at Line %d: Redefined Variable '%s'\n",yylineno,arg->content.c);

            namelist = namelist->next;
            // 插在表头
            p->next = tablehead->next;
            tablehead->next = p;

            printf("newsymbol: %s, tag:%d, type: %s, CurrentLevel: %d\n", tablehead->next->name, tablehead->next->tag, tablehead->next->type, CurrentLevel);
        }
    }
}

// 检测当前块内是否已定义同名变量
bool havedefined(char* name) {
    //printf("------havedefined------------\n");
    struct symbol* p = tablehead->next;
    while(p!=NULL && p->level==CurrentLevel)
    {
        if(!strcmp(p->name,name))
            return true;
        p=p->next;
    }
    return false;
}


// 释放当前level的变量
void 
quitblock() {
    struct symbol* p = tablehead->next, * tp = p;
    if(p == NULL) {
        printf("quitblock: The table is empty.");
        return;
    }
    while(p!=NULL && p->level == CurrentLevel) {
        tp = p->next;
        free(p);
        p = tp;
    }
    tablehead->next = p;
    --CurrentLevel;
    printf("quitblock: CurrentLevel: %d\n", CurrentLevel);
}


// 内->外 逐层检查符号表  检查符号是否已定义 返回其tag值且赋予其type
int 
getdefined(struct astnode* current) {
    struct symbol* p = tablehead->next;
    while(p!=NULL)
    {
        if(!strcmp(p->name,current->content.c)) 
        {
            current->nodetag = p->tag;
            current->type = p->type;
            return p->tag;
        }
        p=p->next;
    }
    return 0;
}

int 
getpnum(struct astnode* current)
{
    struct symbol* p = tablehead->next;
    while(p!=NULL)
    {
        if(!strcmp(p->name,current->content.c)) 
        {
            return p->pnum;
        }
        p = p->next;
    }
    return -1;
}


void 
visitstruct(struct astnode* exp, struct astnode* id) 
{
    struct symbol* p = tablehead->next;
    while(p)
    {
        if(!strcmp(p->name,exp->content.c)) 
        { // exp有定义
            struct symbol* q = p->next;
            while(q)
            {
                if(!strcmp(q->name,exp->type)) 
                { // exp是结构体对象，找到对应结构体q
                    struct para *deflist = q->info.paralist;
                    while(deflist) 
                    {
                        struct names *namelist = deflist->namelist;
                        while(namelist) 
                        {
                            if(!strcmp(namelist->name,id->content.c))
                                return;
                            namelist = namelist->next;
                        }
                        deflist = deflist->next;
                    }
                    printf("Error type 14 at Line %d: Undifined Structure Member ‘%s’\n", yylineno, id->content.c);
                }
                q = q->next;
            }            
        }
        p=p->next;
    }
    printf("Error type 1 at Line %d: Undifined Variable ‘%s’\n", yylineno, exp->content.c);
    return;
}


void 
freetable() {
    struct symbol* p = tablehead->next,  * tp = p;
    while(p->level != CurrentLevel) {
        tp = p->next;
        free(p);
        p = tp;
    }
    free(tablehead);
    printf("\nSymbol table cleaned.\n\n");
}

void 
checkreturn(struct astnode* exp) {
    struct symbol* p = tablehead->next;
    while(p && p->tag==5)
        p=p->next;
    if(!strcmp(p->type, exp->type))
        return ;
    printf("Error type 8 at Line %d: Type Mismatched for Return\n",yylineno);
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
    tablehead = (struct symbol*)malloc(sizeof(struct symbol));
    currentfunction = (struct symbol*)malloc(sizeof(struct symbol));

    //tablehead->next = NULL;    
    rpnum = 0;
    paranum = 0;
    CurrentLevel = 0;
    dimcount = 0;
    tempcount = 0;
    labelcount = 0;
    paramark = 0;
    // 在符号表中预先添加read和write
    struct symbol* read = (struct symbol*)malloc(sizeof(struct symbol));
    struct symbol* write = (struct symbol*)malloc(sizeof(struct symbol));
    read->name = "read";
    read->tag = 5;
    read->pnum = 0;
    read->info.paralist = NULL;
    read->type = "int";
    read->level = 0;
    write->name = "write";
    write->tag = 5;
    write->pnum = 1; 
    write->info.paralist = (struct para*)malloc(sizeof(struct para));
    write->info.paralist->type = "int";
    write->type = "int";
    write->level = 0;

    tablehead->next = read;
    read->next = write;
    write->next = NULL;    

    return yyparse(); //启动文法分析，调用词法分析
}
