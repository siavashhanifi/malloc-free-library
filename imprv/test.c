#include "dlmall.h"

int main(int argc, char const *argv[])
{
    int *mem1 = (int*)dalloc(25);
     printf("%d\n", flist_len()); //1
    int *mem2 = (int*)dalloc(40);
    dfree(mem1);
    printf("%d\n", flist_len()); //2
    int *mem3 = (int*)dalloc(100);
    printf("%d\n", flist_len());
    dfree(mem3);
    int *mem4 = (int*)dalloc(120);
    printf("%d\n", flist_len());
    int *mem5 = (int*)dalloc(10);
    dfree(mem5);
    //print_blocks();

    sanity();
    return 0;
}
