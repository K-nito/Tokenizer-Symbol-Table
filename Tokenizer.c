#include<stdio.h>
#include<string.h>

typedef struct{
    int row;
    int col;
    char type[30];
    char lexeme[100];
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

int isAlpha(char c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z'); }
int isDigit(char c){ return c>='0'&&c<='9'; }
int isAlnum(char c){ return isAlpha(c)||isDigit(c); }

int isDatatype(char *s){
    char *d[]={"int","float","double","char","void",NULL};
    for(int i=0;d[i];i++)
        if(!strcmp(d[i],s)) return 1;
    return 0;
}

int isKeyword(char *s){
    char *k[]={"if","else","while","for","return","break","continue",NULL};
    for(int i=0;k[i];i++)
        if(!strcmp(k[i],s)) return 1;
    return 0;
}

int existsGlobal(char *name){
    for(int i=0;i<gCount;i++)
        if(!strcmp(globalTable[i].name,name))
            return 1;
    return 0;
}

int existsLocal(char *fname,char *name){
    for(int i=0;i<fCount;i++)
        if(!strcmp(functions[i].fname,fname))
            for(int j=0;j<functions[i].count;j++)
                if(!strcmp(functions[i].table[j].name,name))
                    return 1;
    return 0;
}

void addGlobal(char *name,char *type){
    if(!existsGlobal(name)){
        strcpy(globalTable[gCount].name,name);
        strcpy(globalTable[gCount].datatype,type);
        gCount++;
    }
}

void createFunction(char *name){
    strcpy(functions[fCount].fname,name);
    functions[fCount].count=0;
    fCount++;
}

void addLocal(char *fname,char *name,char *type){
    if(!existsLocal(fname,name)){
        for(int i=0;i<fCount;i++)
            if(!strcmp(functions[i].fname,fname)){
                int c=functions[i].count;
                strcpy(functions[i].table[c].name,name);
                strcpy(functions[i].table[c].datatype,type);
                functions[i].count++;
            }
    }
}

Token getNextToken(){
    Token t;
    char c,buf[200];
    int i,startCol;

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

        if(c=='/'){
            char n=fgetc(fp);
            if(n=='/'){
                while((c=fgetc(fp))!='\n' && c!=EOF);
                row++;
                col=0;
                continue;
            }
            if(n=='*'){
                char p=0;
                while((c=fgetc(fp))!=EOF){
                    if(c=='\n'){ row++; col=0; }
                    if(p=='*' && c=='/') break;
                    p=c;
                }
                continue;
            }
            ungetc(n,fp);
        }

        t.row=row;
        startCol=col;
        t.col=startCol;

        if(c=='"'){
            i=0;
            buf[i++]=c;
            while((c=fgetc(fp))!=EOF){
                col++;
                buf[i++]=c;
                if(c=='"') break;
                if(c=='\n'){ row++; col=0; }
            }
            buf[i]='\0';
            strcpy(t.lexeme,buf);
            strcpy(t.type,"STRING_LITERAL");
            return t;
        }

        if(isAlpha(c)||c=='_'){
            i=0;
            buf[i++]=c;
            while(isAlnum(c=fgetc(fp))||c=='_'){
                buf[i++]=c;
                col++;
            }
            buf[i]='\0';
            ungetc(c,fp);

            strcpy(t.lexeme,buf);

            if(isDatatype(buf)) strcpy(t.type,"DATATYPE");
            else if(isKeyword(buf)) strcpy(t.type,"KEYWORD");
            else strcpy(t.type,"IDENTIFIER");

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

        t.lexeme[0]=c;
        t.lexeme[1]='\0';
        strcpy(t.type,"SYMBOL");
        return t;
    }

    strcpy(t.type,"EOF");
    return t;
}

int main(){
    Token t,next;
    char currentType[20]="";
    char currentFunction[50]="";
    int insideFunction=0;
    int braceLevel=0;

    fp=fopen("pptest.c","r");
    if(!fp) return 1;

    while(strcmp((t=getNextToken()).type,"EOF")){

        printf("Row:%d  Col:%d  Type:%-15s  Lexeme:%s\n",
               t.row,t.col,t.type,t.lexeme);

        if(!strcmp(t.type,"DATATYPE")){
            strcpy(currentType,t.lexeme);
            continue;
        }

        if(!strcmp(t.type,"IDENTIFIER")){

            next=getNextToken();

            if(!strcmp(next.lexeme,"(") && strlen(currentType)>0){

                addGlobal(t.lexeme,currentType);
                createFunction(t.lexeme);
                strcpy(currentFunction,t.lexeme);
                insideFunction=1;
                braceLevel=1;

                while(strcmp((next=getNextToken()).lexeme,")")){
                    if(!strcmp(next.type,"DATATYPE"))
                        strcpy(currentType,next.lexeme);
                    else if(!strcmp(next.type,"IDENTIFIER") && strlen(currentType)>0){
                        addLocal(currentFunction,next.lexeme,currentType);
                        currentType[0]='\0';
                    }
                }

                currentType[0]='\0';
            }
            else{
                if(strlen(currentType)>0){
                    if(!insideFunction)
                        addGlobal(t.lexeme,currentType);
                    else
                        addLocal(currentFunction,t.lexeme,currentType);
                    currentType[0]='\0';
                }
                ungetc(next.lexeme[0],fp);
            }
        }

        if(!strcmp(t.lexeme,"{")) braceLevel++;
        if(!strcmp(t.lexeme,"}")){
            braceLevel--;
            if(braceLevel==0){
                insideFunction=0;
                currentFunction[0]='\0';
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
