#include <math.h>
#include<stdlib.h>
#include "rand.h"

#define MAX 16000
#define MIN 16000

int request(){
    double k = log(((double) MAX) / MIN);

    double r = ((double)(rand() % (int)(k*10000))) / 10000;

    int size = (int)((double)MAX / exp(r));
}
