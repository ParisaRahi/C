#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

//b
int stringsum(char *s){

    int strsum = 0;
    int lengde = strlen(s) + 1;
 
    for(int i = 0; i < lengde; i++){
        
        if(isalpha(*s)){ 
            switch (*s)
            {

            case 'L':
                strsum += 12;
                break; 

            case 'l':
                strsum += 12;
                break;

            case 'o':
                strsum += 15;
                break;

            case 'r':
                strsum += 18;
                break;

            case 'e':
                strsum += 5;
                break;

            case 'm':
                strsum += 13;
                break;

            case 'i':
                strsum += 9;
                break;   

            case 'p':
                strsum += 16;
                break;

            case 's':
                strsum += 19;
                break;

            case 'u':
                strsum += 21;
                break;

            case 'd':
                strsum += 4;
                break;

            case 't':
                strsum += 20;
                break;

            case 'a':
                strsum += 1;
                break;
            }
            s++;   
        }
        else if(isspace(*s) || *s == '\0'){
            s++;
        }
        else{
            return -1;
        }
    }
    return strsum;
}

//c
int distance_between(char *s, char c){

    int forekomst = 0;

    char *first = strchr(s, c);

    char *last = strrchr(s,c);

    forekomst = last - first;


    if(first == NULL){
        return -1;
    }

    if(forekomst == 0){
        return 0;
    }

    else{
        return forekomst;
    }
}

//d
char *string_between(char *s, char c){
    char *first = strchr(s, c);
    char *last = strrchr(s, c);


    if(first == NULL){
        return NULL;
    }


    int lengde = (last - first);
    char *substring;

    if (lengde == 0) {
        substring = malloc(sizeof(char) * 1);
        substring[0] = '\0';
        return substring;
    }

    
    substring = malloc(last - first);
    
    strncpy(substring, first+1, lengde-1);
    substring[last - first - 1] = '\0';
    return substring;
}

//e
int stringsum2(char *s, int *res){

    int *strsum = res;
    *strsum = stringsum(s);
    if(*strsum > 0){
        return 0;
    }
    else{
        return -1;
    }

}