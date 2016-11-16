#include <stdio.h>

char a[10];

int bs1(char array[10], char c, int l, int h){
    if(l > h){
        return 0;
    }
    if(l == h){
        if(array[l] == c){
            return 1;
        }
        else{
            return 0;
        }
    }
    if(array[(l+h)/2] == c){
        return 1;
    }
    if(array[(l+h)/2] > c){
        return bs1(array, c, l, (l+h)/2);
    }
    if(array[(l+h)/2 < c){
        return bs1(array, c, (l+h)/2+1, h);
    }
    return 0;   
}

int bs2(char array[10], char c, int l, int h){
    if(l > h){
        return 0;
    }
    while(l != h){
        if(array[(l+h)/2] == c){
            return 1;
        }
        if(array[(l+h)/2 > c){
            h = (l+h)/2;
        }
        else{
            l = (l+h)/2;
        }
    }
    if(array[l] == c){
        return 1;
    }
    else{
        return 0;
    }
    return 0;
}

int main()
{
    int i;
    char c;
    i = 0;
    c = 'a';
    while(i < 10){
        a[i] = c;
        i ++;
        c ++;
    }
    print_i(bs1('d'));
    print_i(bs2('c'));
    return 0;
}

