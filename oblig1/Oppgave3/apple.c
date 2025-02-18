#include<stdio.h>
#include "the_apple.c"
#include<string.h>

//indekstellingen inkluderer både spaces og "\n". 
int locateworm(char *apple){
    int markIndeks = 0;

    int len = 1;
    for(int i = 0; apple[i] != '\0'; i++){
       len++;
    }

    for(int i = 0 ; i < len; i++){
        if(*apple == 'w'){
            markIndeks = i;
        }
        apple++;
    }
    markIndeks++;
    printf("markIndex: %d\n", markIndeks);

    return markIndeks;    
}

int removeworm(char *apple){

    int markLengde = 0;

    int len = 1;
    for(int i = 0; apple[i] != '\0'; i++){
       len++;
    }

    for(int i = 0; i < len; i++){
        if(apple[i] == 'w'||
           apple[i] == 'o'||
           apple[i] == 'r'||
           apple[i] == 'm')
           {
            apple[i] = ' ';
            markLengde++;
           }
    }

    printf("%s", apple);

    printf("markLengde: %d\n", markLengde);

    return markLengde;

}

int main(void){

    locateworm(apple);
    removeworm(apple);

    return 0;
}