#include "allocation.h"
#include "inode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

//id -> 4 bytes
//navn_lengde -> 4 bytes(siste byten sier hvor langt er navnet)
//*navn
//is_diresktory-> 1 byte->(0) for fil, (1) for katalog
//is readonly -> 1 byte -> (0) -> ingen betydning om å løse oppgaven, bestemmer om filen eller katalogen er lesbar og skrivbar eller kun lesbar
//file_size -> 4 bytes,filesize i byte, katalogsize er alltid = 0
//num-entries -> 4 bytes(siste byten sier hvor mye skal leses inn),antall oppførsel, det er (0) så lenge ingen fil eller katalog er opprettet
//*entries -> 8 bytes
//on 32 bit system -> sizeof(uintptr_t) = 4
//on 64 bit system -> sizeof(uintptr_t) = 8


#define BLOCKSIZE 4096
static int unik_id = 0;

struct inode* create_file( struct inode* parent, char* name, char readonly, int size_in_bytes ){
    
    //hvis inode med samme navn finnes fra før
    if(find_inode_by_name(parent, name) != NULL){
        perror("create_file");
        return NULL;
    }

    struct inode *newfile = malloc(sizeof(struct inode));
    if (newfile == NULL) {
        fprintf(stderr, "malloc failed in create_file. possibly out of memory\n");
        exit(EXIT_FAILURE);
    }
    newfile->id = unik_id++;
    newfile->name = strdup(name);
    newfile->is_directory = 0;
    newfile->is_readonly = readonly;
    newfile->filesize = size_in_bytes;

    //TODO:
    /*f->num_entries = ;//hver block har 4096 plass, så for å finne num_entries, må finne ut hvor mange block trenger jeg ift size_in_bytes
    f->entries = ;*/

    int antallblock = ceil(size_in_bytes/BLOCKSIZE);
    newfile->num_entries = antallblock;

    


    
    
    return newfile;
}


struct inode* create_dir( struct inode* parent, char* name ){

    //hvis inode med samme navn finnes fra før
    if(find_inode_by_name(parent, name) != NULL){
        perror("create_dir");
        return NULL;
    }

    struct inode *newdir = malloc(sizeof(struct inode));
    if (newdir == NULL) {
        fprintf(stderr, "malloc failed in create dir. possibly out of memory\n");
        exit(EXIT_FAILURE);
    }

    newdir->id = unik_id++;
    newdir->name = strdup(name);
    newdir->is_directory = 1;
    newdir->is_readonly = 0;
    newdir->filesize = 0;

    //TODO: spørr gruppelærer
    newdir->num_entries = 0;
    newdir->entries = NULL;    

    struct inode *tmp = malloc(sizeof(uintptr_t) * parent->num_entries++ ); 
    //kopiere alle inode fra parent til tmp
    for(int i = 0; i < parent->num_entries; i++){
        tmp[i] = parent[i]; //hvorfor parent->entries[i] ikke fungere?
    }

    //legger den nye inode(newdir) på slutten av arrayet
    parent->entries[parent->num_entries] = (uintptr_t) newdir;
    parent->num_entries++;

    free(tmp);
    return newdir;
}

struct inode* find_inode_by_name( struct inode* parent, char* name ){

    //parent is directory
    if(parent->is_directory){
        for (int i = 0; i < parent->num_entries; i++){
            struct inode *child = (struct inode *) parent->entries[i];
            if(strcmp(child->name , name) == 0){
                return child;
            }   
        }
    }    
    //parent is file
    if(parent->is_directory == 0){
        return NULL;
    }

    //if parent is empty
    if(parent == NULL){
        return NULL;
    }

    return NULL;
}


struct inode* load_inodes(){

    FILE *fil;
    fil = fopen("master_file_table", "rb");

    if(fil == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    struct inode **nodeliste = malloc(sizeof(struct inode *) * 1);
    
    int readcounter;
    int nodeListeIndex = 0;
    int id ;

    while(fread(&id, sizeof(int), 1, fil)){
        struct inode *node = malloc(sizeof(struct inode));
        int name_len = 0;
        node->id = id;
        fread(&name_len, sizeof(int), 1,fil);
        node->name = malloc(sizeof(char) *name_len);
        fread(node->name, sizeof(char),name_len, fil);
        fread(&node->is_directory, sizeof(int), 1, fil);
        fread(&node->is_readonly, sizeof(char), 1, fil);
        fread(&node->num_entries, sizeof(int), 1, fil);
        node->entries = malloc(sizeof(uintptr_t) * node->num_entries);
        readcounter = fread(node->entries, sizeof(uintptr_t), sizeof(node->num_entries), fil);
        nodeliste = realloc(node, sizeof(struct inode *) * (nodeListeIndex + 1));
        nodeliste[nodeListeIndex++] = node;
    }

    for(int i= 0; i < nodeListeIndex; i++){
        struct inode *ino = nodeliste[i];
        //true -> non-zero
        //false -> 0
        //hvis inoden er katalog
        if(ino->is_directory && ino->entries != NULL){
            for(int j = 0; j < ino->num_entries; j++){//j -> er entries
               int id = (int)ino->entries[j];
                for(int k = 0; k < nodeListeIndex; k++){//k er
                   // int ino_id = (int)ino->entries;
                    if(id == nodeliste[k]->id){
                        nodeliste[k] = (struct inode *)ino->entries[j];
                    }
                }
            }
        }
    }

    /*struct inode *root = malloc(sizeof(struct inode));
    
    //lese master_file_table:
    fread(&root->id, sizeof(int), 1, fil);
    int name_len = 0;
    fread(&name_len, sizeof(int), 1, fil);

    root->name = malloc(sizeof(char) * name_len);
    fread(root->name,sizeof(char), name_len, fil);

    fread(&root->is_directory, sizeof(int), 1, fil);

    fread(&root->is_readonly, sizeof(char), 1, fil);
    fread(&root->num_entries, sizeof(int), 1, fil);

    root->entries = malloc(sizeof(uintptr_t) * root->num_entries);
    fread(root->entries, sizeof(uintptr_t), sizeof(root->num_entries), fil);

    //hvis entries er en katalog
    if(root->is_directory){
        

    }

    //ellers er en fil
    //hver entries i directory inneholder en id til en annen inode
    //så bør opprettes en ny inode som kan enten være en fil eller en directory
    else{

    }*/

 

    fclose(fil);
    return NULL;
}

//frigjøre alle inode_datastrukturer
void fs_shutdown( struct inode* inode ){

}

/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;

void debug_fs( struct inode* node )
{
    if( node == NULL ) return;
    for( int i=0; i<indent; i++ )
        printf("  ");
    if( node->is_directory )
    {
        printf("%s (id %d)\n", node->name, node->id );
        indent++;
        for( int i=0; i<node->num_entries; i++ )
        {
            struct inode* child = (struct inode*)node->entries[i];
            debug_fs( child );
        }
        indent--;
    }
    else
    {
        printf("%s (id %d size %db blocks ", node->name, node->id, node->filesize );
        for( int i=0; i<node->num_entries; i++ )
        {
            printf("%d ", (int)node->entries[i]);
        }
        printf(")\n");
    }
}

