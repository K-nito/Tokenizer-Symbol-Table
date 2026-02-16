#include<stdio.h>
#include<stdlib.h>

int multiply(int a, int b) {
        return a * b;
}

int pow(int a, int x) {
        if (x==0) return 1;
        return a * pow(a,x-1);
}

int main()
{
        int num;
        printf("Enter a number: ");
        scanf("%d",&num);
        if(num<0) printf("Negative");
        else if(num>0) printf("Positive");
        else printf("Zero");
        return 0;
}
