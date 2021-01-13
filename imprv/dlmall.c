#include "dlmall.h"

struct head {
    uint16_t bfree;
    uint16_t bsize;
    uint16_t free;
    uint16_t size;
    struct head *next;
    struct head *prev;
};

struct taken {
    uint16_t bfree;
    uint16_t bsize;
    uint16_t free;
    uint16_t size;
};

#define TRUE 1
#define FALSE 0


#define HEAD (sizeof(struct taken)) //changed
#define MIN(size) (((size)>(16))?(size):(16)) //changed
#define LIMIT(size) (MIN(0) + HEAD + size)
#define MAGIC(memory) ((struct taken*)memory - 1)
#define HIDE(block) (void*)((struct taken*)block + 1)
#define ALIGN 8
#define ARENA (64*1024)


void print_block();

/*after and before*/


struct head *after(struct head *block){
    return (struct head*)((char*)block + HEAD + block->size);
}

struct head *before(struct head *block){
    return (struct head*)((char*)block - block->bsize - HEAD);
}

/* split a block */
struct head *split(struct head *block, int size){
    int rsize = block->size - size - HEAD;
    block->size = rsize;

    struct head *splt =(struct head*)((char*)block + HEAD + rsize);
    splt->bsize = rsize;
    splt->bfree = TRUE;
    splt->free = TRUE;
    splt->size = size;
    
    struct head *aft = after(splt);
    aft->bsize = size;

    return splt;
}

/* a new block */

struct head *arena = NULL;

struct head *new(){
    if(arena != NULL){
        printf("one arena already allocated \n");
        return NULL;
    }

    struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(new == MAP_FAILED){
        printf("mmap failed: error%d\n", 123);
        return NULL;
    }

    uint16_t size = ARENA - 2*HEAD;

    new->bfree = FALSE;
    new->bsize = 0;
    new->size = size;
    new->free = TRUE;

    struct head *sentinel = after(new);

    sentinel->bfree = TRUE;
    sentinel->bsize = size;
    sentinel->free = FALSE;
    sentinel->size = 0;

    arena = (struct head*)new;
    printf("\n\n");
  
    return new;
}

/* the free list */

struct head *flist = NULL;

void detach(struct  head *block) {
  
    if(block->next != NULL)
        block->next->prev = block->prev;
    if(block->prev != NULL)
        block->prev->next = block->next;
    else
        flist = block->next;
        

}

void insert(struct head *block){
 
    block->next = flist;
    block->prev = NULL;

    if(flist != NULL)
        flist->prev = block;

    flist = block;
}

/* allocate and free */

int adjust(int bytes){
    int enough_bytes = MIN(bytes);

    if(enough_bytes % ALIGN == 0){
        return enough_bytes;
    }else{
        int offset = enough_bytes;
        int padding = (ALIGN - (offset % ALIGN)) % ALIGN;
        return offset + padding;
    }
}

struct head *find(int bytes){
    struct head *iterator = flist;
    while(iterator != NULL){
        int size = iterator->size;
        if(size >= LIMIT(bytes)){
            detach(iterator);
            struct head *splt = split(iterator, bytes);
            splt->free = FALSE;
            after(splt)->bfree = FALSE;
            struct head *bfr = before(splt);
            insert(bfr);
            return splt;
        }
        else if(size < LIMIT(bytes) && size >= bytes){
            iterator->free = FALSE;
            detach(iterator);
            after(iterator)->bfree = FALSE;
            return iterator;
        } 
        iterator = iterator->next;
    }
    return NULL;
}

struct head *merge(struct head *block){
    struct head *aft = after(block);
    if(block->bfree){
        struct head *bfr= before(block);
        detach(bfr);
        bfr->size += block->size + HEAD;
        aft->bsize = bfr->size;
        block = bfr;
    }

    if(aft->free){
        detach(aft);
        block->size += aft->size + HEAD;
        aft = after(aft);
        aft->bsize = block->size;
    }

    return block;
}

void *dalloc(size_t request){
    if(request <= 0)
        return NULL;
    if(arena == NULL){
        insert(new());
    }
    int size = adjust(request);
    struct taken *taken = find(size);
    if(taken == NULL)
        return NULL;
    else
        return HIDE(taken);
}

void dfree_nonmerge(void *memory){
    if(memory != NULL){
        struct head *block = (struct head*)MAGIC(memory);
        struct head *aft = after(block);
        block->free = TRUE;
        aft->bfree = TRUE;
        insert(block);
    }
    return;
}

void dfree(void *memory){
    if(memory != NULL){
        struct head *block = (struct head*)MAGIC(memory); //added typecast
        block = merge(block);
        struct head *aft = after(block);
        block->free = TRUE;
        aft->bfree = TRUE;
        insert(block);
    }
}

/*utility*/

int free_list_ok(){
    struct head *iterator = flist;
    while (iterator != NULL){
        if(!iterator->free)
            return FALSE;
        iterator = iterator->next;
   }
   return TRUE;
}

int segment_consistent(){
    struct head *iterator = arena;
    struct head *aft = after(arena);
    
    int prev_size = 0;
    int prev_free = 0;
    while (aft->size != 0){
        prev_size = iterator->size;
        prev_free = iterator->free;
        if(prev_size != aft->bsize && prev_free != aft->bfree){
            printf("prev_size: %d ", prev_size);
            printf("prev_free: %d\n", prev_free);
            printf("bsize: %d ", aft->bsize);
            printf("bfree: %d\n", aft->bfree);
            /*print_block(before(aft));
            print_block(aft);*/
            return FALSE;
        }
        iterator = after(iterator);
        aft = after(iterator);
   }
   prev_size = before(aft)->size;
   prev_free = before(aft)->free;
    if(prev_size != aft->bsize && prev_free != aft->bfree){
            printf("prev_size: %d ", prev_size);
            printf("prev_free: %d\n", prev_free);
            printf("bsize: %d ", aft->bsize);
            printf("bfree: %d\n", aft->bfree);
        print_block(before(aft));
        print_block(aft);
        return FALSE;
    }

   return TRUE;
}

void sanity(){
    int flist_ok = free_list_ok();
    int seg_ok = segment_consistent();
    if(flist_ok)
        printf("free list ok :)\n");
    else
        printf("free list not well :(\n");
    
    if(seg_ok){
        printf("segment consistent :)");
    }else
        printf("segment inconsistent :(");
}

/* benchmarking tools */

int flist_len(){
    struct head *iterator = flist;
    int i = 0;
    while(iterator != NULL){
        i++;
        iterator = iterator->next;
    }
    return i;
}

void print_flist_bs(){
    struct head *iterator = flist;
    while(iterator != NULL){
        printf("[%d], ", iterator->size);
        iterator = iterator->next;
    }
}


/* debugging tools */

void print_blocks(){
        struct head *block = arena; 
        while(after(block)->size != 0){
        printf("*******BLOCK_START*******\n");
        printf("head ptr addr: %p\n", block);
        printf("bfree: %d\n", block->bfree);
        printf("bsize: %d\n", block->bsize);
        printf("size: %d\n", block->size);
        printf("free: %d\n", block->free);
        printf("*******BLOCK_END*******\n");
        block = after(block);
        }
}

void print_block(struct head *block){
        printf("*******BLOCK_START*******\n");
        printf("head ptr addr: %p\n", block);
        printf("bfree: %d\n", block->bfree);
        printf("bsize: %d\n", block->bsize);
        printf("size: %d\n", block->size);
        printf("free: %d\n", block->free);
        printf("*******BLOCK_END*******\n");
}

/*testing*/

void test_flist(){
    struct head *itr = flist;
    
    printf("head of flist: \n");
    while (itr != NULL){
        print_block(itr);
        itr = itr->next;
    }
    printf("reached end of flist\n");

}

/*int main(){
    test_flist();
}*/