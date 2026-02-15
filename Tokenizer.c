#include<stdio.h>
#include<string.h>

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

Symbol globalTable[100];
int gCount=0;

FunctionTable functions[50];
int fCount=0;

int isAlpha(char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z');}
int isDigit(char c){return c>='0'&&c<='9';}
int isAlnum(char c){return isAlpha(c)||isDigit(c);}

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

void addGlobal(char *name,char *type){
    strcpy(globalTable[gCount].name,name);
    strcpy(globalTable[gCount].datatype,type);
    gCount++;
}

void createFunction(char *name){
    strcpy(functions[fCount].fname,name);
    functions[fCount].count=0;
    fCount++;
}

void addLocal(char *fname,char *name,char *type){
    for(int i=0;i<fCount;i++){
        if(!strcmp(functions[i].fname,fname)){
            int c=functions[i].count;
            strcpy(functions[i].table[c].name,name);
            strcpy(functions[i].table[c].datatype,type);
            functions[i].count++;
        }
    }
}

void skipString(){
    char c;
    while((c=fgetc(fp))!='"' && c!=EOF);
}

void skipPreprocessor(){
    char c;
    while((c=fgetc(fp))!='\n' && c!=EOF);
}

void skipComment(){
    char c,p=0;
    while((c=fgetc(fp))!=EOF){
        if(p=='*' && c=='/') break;
        p=c;
    }
}

int main(){
    char c,word[100],currentType[20]="";
    char currentFunction[50]="";
    int insideFunction=0;
    int i;

    fp=fopen("pptest.c","r");
    if(!fp) return 1;

    while((c=fgetc(fp))!=EOF){

        if(c=='#'){ skipPreprocessor(); continue; }

        if(c=='"'){ skipString(); continue; }

        if(c=='/'){
            char n=fgetc(fp);
            if(n=='*'){ skipComment(); continue; }
            if(n=='/'){ skipPreprocessor(); continue; }
            ungetc(n,fp);
        }

        if(isAlpha(c)){
            i=0;
            word[i++]=c;
            while(isAlnum(c=fgetc(fp))){ word[i++]=c; }
            word[i]='\0';
            ungetc(c,fp);

            if(isDatatype(word)){
                strcpy(currentType,word);
                continue;
            }

            if(isKeyword(word)){
                continue;
            }

            if(strlen(currentType)>0){
                char next=fgetc(fp);

                if(next=='('){
                    addGlobal(word,currentType);
                    createFunction(word);
                    strcpy(currentFunction,word);
                    insideFunction=1;

                    while((c=fgetc(fp))!=')'){
                        if(isAlpha(c)){
                            i=0;
                            word[i++]=c;
                            while(isAlnum(c=fgetc(fp))) word[i++]=c;
                            word[i]='\0';
                            ungetc(c,fp);

                            if(isDatatype(word))
                                strcpy(currentType,word);
                            else if(strlen(currentType)>0){
                                addLocal(currentFunction,word,currentType);
                                currentType[0]='\0';
                            }
                        }
                    }
                }
                else{
                    ungetc(next,fp);
                    if(!insideFunction)
                        addGlobal(word,currentType);
                    else
                        addLocal(currentFunction,word,currentType);
                }
                currentType[0]='\0';
            }
        }

        if(c=='}'){
            insideFunction=0;
            currentFunction[0]='\0';
        }
    }

    printf("\nGLOBAL SYMBOL TABLE\n");
    printf("Name\tDatatype\n");
    for(i=0;i<gCount;i++)
        printf("%s\t%s\n",globalTable[i].name,globalTable[i].datatype);

    for(i=0;i<fCount;i++){
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
