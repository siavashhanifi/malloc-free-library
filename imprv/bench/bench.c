#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include"../dlmall.h"

#define BYTES 16
#define BLOCKS 1000
#define ROUNDS 100
int main(){
    clock_t start;
    clock_t end;

    if(BYTES * BLOCKS > 64*1024 - 24*2){
        printf("not enough space");
    }

    int *blocks[BLOCKS];
    start = clock();
    for(int i = 0; i < BLOCKS; i++){
        blocks[i] = (int*)dalloc(BYTES);
    }
    
    for(int j = 0; j<= ROUNDS; j++ ){
        for(int i = 0; i < BLOCKS; i++){
            *blocks[i] = 1;
        }
    }
    end = clock() - start;
    printf("%ld\n", end);
    //sanity();
    return 0;
}
