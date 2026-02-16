#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 200

FILE *fp;
int row = 1, col = 0;

typedef struct{
    char lexeme[100];
    char type[50];
    int row;
    int col;
}Token;

typedef struct{
    char name[50];
    char datatype[50];
}Symbol;

Symbol globalTable[MAX];
Symbol localTable[20][MAX];

int gCount = 0;
int lCount[20] = {0};
int funcIndex = -1;

char currentDatatype[20] = "";
char currentFunction[20] = "";

int isLetter(char c){
    if(c>='a' && c<='z') return 1;
    if(c>='A' && c<='Z') return 1;
    return 0;
}

int isDigit(char c){
    if(c>='0' && c<='9') return 1;
    return 0;
}

int isKeyword(char *str){
    char *kw[]={"if","else","while","for","return","break","continue"};
    for(int i=0;i<7;i++)
        if(strcmp(str,kw[i])==0) return 1;
    return 0;
}

int isDatatype(char *str){
    char *dt[]={"int","float","char","double","void"};
    for(int i=0;i<5;i++)
        if(strcmp(str,dt[i])==0) return 1;
    return 0;
}

int exists(Symbol table[],int count,char *name){
    for(int i=0;i<count;i++)
        if(strcmp(table[i].name,name)==0)
            return 1;
    return 0;
}

void addGlobal(char *name,char *datatype){
    if(!exists(globalTable,gCount,name)){
        strcpy(globalTable[gCount].name,name);
        strcpy(globalTable[gCount].datatype,datatype);
        gCount++;
    }
}

void addLocal(char *name,char *datatype){
    if(funcIndex==-1) return;
    if(!exists(localTable[funcIndex],lCount[funcIndex],name)){
        strcpy(localTable[funcIndex][lCount[funcIndex]].name,name);
        strcpy(localTable[funcIndex][lCount[funcIndex]].datatype,datatype);
        lCount[funcIndex]++;
    }
}

Token getNextToken(){
    Token t;
    char c,buf[100];
    int i;

    while((c=fgetc(fp))!=EOF){

        col++;

        if(c=='\n'){
            row++;
            col=0;
            continue;
        }

        if(c==' '||c=='\t')
            continue;

        if(c=='#'){
            while((c=fgetc(fp))!='\n' && c!=EOF);
            row++;
            col=0;
            continue;
        }

        t.row=row;
        t.col=col;

        if(c=='"'){
            i=0;
            buf[i++]=c;
            while((c=fgetc(fp))!='"' && c!=EOF){
                buf[i++]=c;
                col++;
            }
            buf[i++]='"';
            buf[i]='\0';
            strcpy(t.lexeme,buf);
            strcpy(t.type,"STRING_LITERAL");
            return t;
        }

        if(isLetter(c) || c=='_'){
            i=0;
            buf[i++]=c;
            while(isLetter(c=fgetc(fp)) || isDigit(c) || c=='_'){
                buf[i++]=c;
                col++;
            }
            buf[i]='\0';
            ungetc(c,fp);

            strcpy(t.lexeme,buf);

            if(isDatatype(buf))
                strcpy(t.type,"DATATYPE");
            else if(isKeyword(buf))
                strcpy(t.type,"KEYWORD");
            else
                strcpy(t.type,"IDENTIFIER");

            return t;
        }

        if(isDigit(c)){
            i=0;
            buf[i++]=c;
            while(isDigit(c=fgetc(fp))){
                buf[i++]=c;
                col++;
            }
            buf[i]='\0';
            ungetc(c,fp);
            strcpy(t.lexeme,buf);
            strcpy(t.type,"NUMBER");
            return t;
        }

        char next=fgetc(fp);
        char op[3];
        op[0]=c;
        op[1]=next;
        op[2]='\0';

        if(strcmp(op,"==")==0 || strcmp(op,"!=")==0 ||
           strcmp(op,"<=")==0 || strcmp(op,">=")==0 ||
           strcmp(op,"&&")==0 || strcmp(op,"||")==0){
            strcpy(t.lexeme,op);
            strcpy(t.type,"OPERATOR");
            col++;
            return t;
        }

        ungetc(next,fp);

        if(c=='+'||c=='-'||c=='*'||c=='/'||c=='%'||
           c=='='||c=='<'||c=='>'||c=='!'){
            t.lexeme[0]=c;
            t.lexeme[1]='\0';
            strcpy(t.type,"OPERATOR");
            return t;
        }

        if(c=='('||c==')'||c=='{'||c=='}'||c==';'||c==','){
            t.lexeme[0]=c;
            t.lexeme[1]='\0';
            strcpy(t.type,"SYMBOL");
            return t;
        }
    }

    strcpy(t.type,"EOF");
    return t;
}

int main(){

    Token t;
    fp=fopen("input.c","r");

    if(!fp){
        printf("File not found\n");
        return 0;
    }

    printf("TOKENS\n\n");

    while(strcmp((t=getNextToken()).type,"EOF")!=0){

        printf("Row:%d Col:%d Type:%-15s Lexeme:%s\n",
               t.row,t.col,t.type,t.lexeme);

        if(strcmp(t.type,"DATATYPE")==0){
            strcpy(currentDatatype,t.lexeme);
        }

        else if(strcmp(t.type,"IDENTIFIER")==0){

            char next=fgetc(fp);
            ungetc(next,fp);

            if(next=='(' && strcmp(currentDatatype,"")!=0){
                addGlobal(t.lexeme,currentDatatype);
                funcIndex++;
                strcpy(currentFunction,t.lexeme);
            }
            else{
                if(funcIndex==-1)
                    addGlobal(t.lexeme,currentDatatype);
                else
                    addLocal(t.lexeme,currentDatatype);
            }
        }
    }

    printf("\nGLOBAL SYMBOL TABLE\n");
    printf("Name\tDatatype\n");
    for(int i=0;i<gCount;i++)
        printf("%s\t%s\n",globalTable[i].name,globalTable[i].datatype);

    for(int i=0;i<=funcIndex;i++){
        printf("\nLOCAL SYMBOL TABLE FOR FUNCTION %s\n",globalTable[i].name);
        printf("Name\tDatatype\n");
        for(int j=0;j<lCount[i];j++)
            printf("%s\t%s\n",localTable[i][j].name,localTable[i][j].datatype);
    }

    fclose(fp);
    return 0;
}
