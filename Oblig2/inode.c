# include "allocation.h"
# include "inode.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <math.h>


# define BLOCKSIZE 4096
static int unik_id = 0;

struct inode *create_file(struct inode *parent, char *name, char readonly, int size_in_bytes){

    //hvis inode med samme navn finnes fra før
    if(find_inode_by_name(parent, name) != NULL){
        perror("create_file");
        return NULL;
    }

    /*if(parent->is_directory == 0){
        return NULL;
    }*/

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

    //antall block som trengs for en fil
    int antallblock = ceil(size_in_bytes/BLOCKSIZE);
    newfile->num_entries = antallblock;


    newfile->entries = malloc(sizeof(uintptr_t) * antallblock );
    if (newfile->entries == NULL) {
        fprintf(stderr, "tmp malloc failed in create_file. possibly out of memory\n");
        exit(EXIT_FAILURE);
    }    
    for(int i = 0; i < newfile->num_entries; i++){
        newfile->entries[i] = (uintptr_t)allocate_block();
        if((int)newfile->entries[i] == -1){
            for(int j = 0; j < i; j++){
                free_block(newfile->entries[j]);
            }
            break;
        }
    }


    if(parent == NULL){
        return newfile;
    }

    //hvis parent ikke er NULL
    parent->num_entries++;
    uintptr_t *tmp = malloc(sizeof(uintptr_t) * (parent->num_entries ) );
        if (tmp == NULL) {
        fprintf(stderr, "tmp malloc failed in create_file. possibly out of memory\n");
        exit(EXIT_FAILURE);
    }

    //kopiere alle inode fra parent til tmp(den nye entries)
    for(int i = 0; i < parent->num_entries-1; i++){
        tmp[i] = parent->entries[i];
    }

    //legger den nye inode(newdir) på slutten av arrayet
    tmp[parent->num_entries-1] = (uintptr_t) newfile;

    //frigjøre den som vi hadde før, så får den ny verdi
    free(parent->entries);
    parent->entries = tmp;


    return newfile;
}


struct inode *create_dir( struct inode *parent, char* name ){

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


    //TODO:
    newdir->num_entries = 0;
    newdir->entries = NULL;
    if(parent == NULL){
        printf("parent er NULL\n");
        return newdir;
    }

    //hvis parent ikke er NULL
    //opprette en ny entries
    parent->num_entries++;
    uintptr_t *tmp = malloc(sizeof(uintptr_t) * (parent->num_entries ) );
        if (tmp == NULL) {
        fprintf(stderr, "tmp malloc failed in create dir. possibly out of memory\n");
        exit(EXIT_FAILURE);
    }

    //kopiere alle inode fra parent til tmp(den nye entries)
    for(int i = 0; i < parent->num_entries-1; i++){
        tmp[i] = parent->entries[i];
    }

    //legger den nye inode(newdir) på slutten av arrayet
    tmp[parent->num_entries-1] = (uintptr_t) newdir;

    //frigjøre den som vi hadde før, så får den ny verdi
    free(parent->entries);
    parent->entries = tmp;
    
    return newdir;

}

struct inode *find_inode_by_name( struct inode *parent, char *name ){

    if(parent == NULL){
        return NULL;
    }
    //parent is directory
    if(parent->is_directory){
        for (int i = 0; i < parent->num_entries; i++){
            struct inode *child = (struct inode *) parent->entries[i];
            if(strcmp(child->name , name) == 0){
                return child;
            }
        }
    }

    return NULL;
}


struct inode *load_inodes(){

    FILE *fil;
    fil = fopen("master_file_table", "rb");

    if(fil == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    struct inode **nodeliste;//allokerte plass til en inode, så for å legge til mer enn en inode, så må den reallokes

    
    int nodeListeIndex = 0;
    int id;
    
    while(fread(&id, sizeof(int),1, fil)){
        
        struct inode *node = malloc(sizeof(struct inode));
        node->id = id;

        int name_len = 0;
        fread(&name_len, sizeof(int), 1,fil);

        node->name = malloc(sizeof(char) * name_len);
        fread(node->name, sizeof(char),name_len, fil);

        fread(&node->is_directory, sizeof(char), 1, fil);

        fread(&node->is_readonly, sizeof(char), 1, fil);

        fread(&node->filesize, sizeof(int), 1, fil);
 
        fread(&node->num_entries, sizeof(int), 1, fil);

        node->entries = malloc(sizeof(uintptr_t) * (node->num_entries));
        fread(node->entries, sizeof(uintptr_t), node->num_entries, fil);
 
        if(nodeListeIndex == 0){
            nodeliste = malloc(sizeof(struct inode*) );
        }
        else{
            nodeliste = realloc(nodeliste, (sizeof(struct inode *) * (nodeListeIndex + 1)));
        }
        if (nodeliste){
            nodeliste[nodeListeIndex] = node;

        }
        else {
            printf("Couldnt realloc/malloc nodelist\n");
            exit(1);
        }      
        nodeListeIndex++;

    }

    for(int i= 0; i < nodeListeIndex; i++){

        struct inode *ino = nodeliste[i];

        //hvis inoden er katalog
        if(ino->is_directory){
            for(int j = 0; j < ino->num_entries; j++){//j -> er entries
                int id = (int)ino->entries[j];
                for(int k = 0; k < nodeListeIndex; k++){
                    if(id == nodeliste[k]->id){
                        ino->entries[j] = (uintptr_t) nodeliste[k];
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
    //velger inode i indeks 0 som rootnoden
    struct inode *rootinode = nodeliste[0];
    free(nodeliste);

    fclose(fil);
    return rootinode;
}

//frigjøre alle inode_datastrukturer
void fs_shutdown( struct inode *inode ){
    if(!inode) return;
    //i hver inode må inode->name, inode->entries, inode frigjøres
    //hvis inode er katalog
    if(inode->is_directory){
        for(int i = 0; i < inode->num_entries; i++){
            struct inode *childe = (struct inode *) inode->entries[i];
            fs_shutdown(childe);
        }
    }
    if (inode->name) free(inode->name);
    if (inode->entries) free(inode->entries);
    free(inode);

}

/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;

void debug_fs( struct inode *node )
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
