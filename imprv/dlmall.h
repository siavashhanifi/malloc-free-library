#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

void *dalloc(size_t bytes);
void dfree(void *memory);
void dfree_nonmerge(void *memory);
void sanity();
void print_blocks();
void test_flist();
int flist_len();
void print_flist_bs();