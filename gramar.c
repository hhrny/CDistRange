#include <stdio.h>

int fun(char c){
    int tmp = (int)c;
    int tmp2 = tmp + tmp;
    return tmp2/tmp+1-3*(-tmp);
}

int main(){
    int n = 0;
    char c = 'a';
    while(n++<10){
        if(fun(c) > 10){
            printf("hello ");
        }
        else{
            printf("world !\n");
        }
        c ++;
    }
    printf("\n");
    return 0;
}
