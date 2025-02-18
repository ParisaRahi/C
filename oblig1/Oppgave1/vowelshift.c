#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

    //for å unngå segment fault
    if(argc < 3){
        printf("OPPGI STRING OG VOKAL\n");
        return 1;
    }

    /*printf("argc: %d\n", argc);
    for(int i= 1; i < argc; i++){
        printf("argv[ %d ] = %s\n", i, argv[i]);
    }
    int length = strlen(argv[1]);
    printf("length of argv[1]: %d\n", length);*/

    
    char *streng = argv[1];
    char *newStreng = malloc(strlen(streng) + 1);
    char *vowel = argv[2];

    if( newStreng == NULL){
        fprintf(stderr, "malloc feil i newStreng\n");
        exit(1);
    }

    int lengde = strlen(streng) + 1;

    for(int i = 0; i <lengde ; i++){
        if(tolower(streng[i]) == 'a' ||
            tolower(streng[i]) == 'e'||
            tolower(streng[i]) == 'i'||
            tolower(streng[i]) == 'o'||
            tolower(streng[i] == 'u') )
            {
            newStreng[i] = *vowel; //0x4532059h123 -> 'a'
            }

        else{
            newStreng[i] = streng[i];
        }    
    }
    printf("%s\n", newStreng);
    free(newStreng);

    return 0;

}