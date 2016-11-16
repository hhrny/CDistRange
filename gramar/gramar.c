#include <stdio.h>

struct A{
    int num;
    char data[8];
    struct A *next;
}

int fun(char c){
    int tmp = (int)c;
    int tmp2 = tmp + tmp;
    return tmp2/tmp+1-3*(-tmp);
}

void fun2(struct A * a){
    (*a).num = 1;
    a->data[0] = 'a';
    a->next = 0;
}

void printA(struct A a){
    int n = 0;
    print_i(a.num);
    while(n<a.num){
        print_c(a.data[n]);
        n++;
    }
}

int main(){
    struct A array[10];
    int n = 0;
    char c = 'a';
    while(n<10){
        array[n].num = 0;
        array[n].next = 0;
        n ++;
        if(fun(c) > 10){
            print_i(n);
        }
        else{
            print_c(c);
        }
    }
    fun2(&array[0]);
    printA(array[0]);
    print_c('n');
    return 0;
}
