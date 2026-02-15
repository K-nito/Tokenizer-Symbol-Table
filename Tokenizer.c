#include <stdio.h>
#include <string.h>

typedef struct{
    int row;
    int col;
    char type[30];
    char lexeme[50];
}Token;

typedef struct{
    char name[50];
    char datatype[20];
}Symbol;

typedef struct{
    char fname[50];
    Symbol table[100];
    int count;
}FunctionTable;

FILE *fp;
int row=1,col=0;

Symbol globalTable[100];
int gCount=0;

FunctionTable functions[50];
int fCount=0;

int isAlpha(char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z');}
int isDigit(char c){return c>='0'&&c<='9';}
int isAlnum(char c){return isAlpha(c)||isDigit(c);}

int isKeyword(char *str){
    char *kw[]={"int","float","double","char","void",NULL};
    for(int i=0;kw[i];i++)
        if(!strcmp(str,kw[i])) return 1;
    return 0;
}

int isSpecialSymbol(char c){
    char s[]=";,(){}";
    for(int i=0;s[i];i++)
        if(c==s[i]) return 1;
    return 0;
}

Token getNextToken(){
    Token t;
    char c,buf[50];
    int i;

    while((c=fgetc(fp))!=EOF){
        col++;
        if(c=='\n'){row++;col=0;continue;}
        if(c==' '||c=='\t') continue;

        t.row=row; t.col=col;

        if(isAlpha(c)||c=='_'){
            i=0; buf[i++]=c;
            while(isAlnum(c=fgetc(fp))||c=='_'){buf[i++]=c;col++;}
            buf[i]='\0';
            ungetc(c,fp);
            strcpy(t.lexeme,buf);
            strcpy(t.type,isKeyword(buf)?"KEYWORD":"IDENTIFIER");
            return t;
        }

        if(isDigit(c)){
            i=0; buf[i++]=c;
            while(isDigit(c=fgetc(fp))){buf[i++]=c;col++;}
            buf[i]='\0';
            ungetc(c,fp);
            strcpy(t.lexeme,buf);
            strcpy(t.type,"NUMBER");
            return t;
        }

        if(isSpecialSymbol(c)){
            t.lexeme[0]=c; t.lexeme[1]='\0';
            strcpy(t.type,"SPECIAL_SYMBOL");
            return t;
        }
    }

    strcpy(t.type,"EOF");
    return t;
}

void addGlobal(char *name,char *datatype){
    strcpy(globalTable[gCount].name,name);
    strcpy(globalTable[gCount].datatype,datatype);
    gCount++;
}

void createFunction(char *name){
    strcpy(functions[fCount].fname,name);
    functions[fCount].count=0;
    fCount++;
}

void addLocal(char *fname,char *name,char *datatype){
    for(int i=0;i<fCount;i++){
        if(!strcmp(functions[i].fname,fname)){
            int c=functions[i].count;
            strcpy(functions[i].table[c].name,name);
            strcpy(functions[i].table[c].datatype,datatype);
            functions[i].count++;
        }
    }
}

int main(){
    Token t,next;
    char currentType[20]="";
    char currentFunction[50]="";
    int braceLevel=0;
    int insideFunction=0;

    fp=fopen("pptest.c","r");
    if(!fp) return 1;

    while(strcmp((t=getNextToken()).type,"EOF")){

        if(!strcmp(t.type,"KEYWORD")){
            strcpy(currentType,t.lexeme);
            continue;
        }

        if(!strcmp(t.type,"IDENTIFIER")){
            next=getNextToken();

            if(!strcmp(next.lexeme,"(")){
                Token temp=next;
                int param=1;
                while(param){
                    temp=getNextToken();
                    if(!strcmp(temp.lexeme,")")) param=0;
                }
                Token after=getNextToken();
                if(!strcmp(after.lexeme,"{")){
                    addGlobal(t.lexeme,currentType);
                    createFunction(t.lexeme);
                    strcpy(currentFunction,t.lexeme);
                    insideFunction=1;
                    braceLevel=1;
                }
            }
            else{
                if(!insideFunction)
                    addGlobal(t.lexeme,currentType);
                else
                    addLocal(currentFunction,t.lexeme,currentType);
            }
        }

        if(!strcmp(t.lexeme,"{")) braceLevel++;
        if(!strcmp(t.lexeme,"}")){
            braceLevel--;
            if(braceLevel==0){
                insideFunction=0;
                strcpy(currentFunction,"");
            }
        }
    }

    printf("\nGLOBAL SYMBOL TABLE\n");
    printf("Name\tDatatype\n");
    for(int i=0;i<gCount;i++)
        printf("%s\t%s\n",globalTable[i].name,globalTable[i].datatype);

    for(int i=0;i<fCount;i++){
        printf("\nLOCAL SYMBOL TABLE FOR FUNCTION %s\n",functions[i].fname);
        printf("Name\tDatatype\n");
        for(int j=0;j<functions[i].count;j++)
            printf("%s\t%s\n",
                functions[i].table[j].name,
                functions[i].table[j].datatype);
    }

    fclose(fp);
    return 0;
}
