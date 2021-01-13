#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include"../dlmall.h"
#include"rand.h"

#define LOOP 5000
#define BUFFER 100

int main(){
    void *buffer[BUFFER];
    for(int i = 0;i < BUFFER; i++){
        buffer[i] = NULL;
    }

    for(int i = 0; i < LOOP; i++){

            int index = rand() % BUFFER;
            if(buffer[index] != NULL){
                dfree_nonmerge(buffer[index]);
            }

            size_t size = (size_t)request();
            int *memory = (int*)dalloc(size);
            if(memory == NULL){
                fprintf(stderr, "dalloc failed, out of memory\n");
                return(1);
            }

            buffer[index] = memory;
            *memory = 123;
            printf("%d\n", flist_len());
            //print_flist_bs();

            
    }
   //sanity();
    return 0;
}