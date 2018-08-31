#ifndef DATASET
#define DATASET

#include "contiki.h"

#define VAL_LEN 6
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef COMMUNICATION
#undef COMMUNICATION
#endif
#define ROOT_NODE 1
#define NUM_SENSORS 1

//const real scaling[VAL_LEN] = {10, 10, 10, 10, 100, 100};
const real scaling[VAL_LEN] = {1000, 1000, 1000, 1000, 1000, 1000};

void getName(char *name)
{
    sprintf(name, NAME(HIWS));
}

int init()
{
    return 1;
}

#endif /* DATASET */
