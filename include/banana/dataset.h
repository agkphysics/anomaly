#ifndef DATASET
#define DATASET

#include "contiki.h"

#define VAL_LEN 2
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>

#define COMMUNICATION
#define ROOT_NODE 1
#define NUM_SENSORS 3

const real scaling[VAL_LEN] = {1000, 1000};

void getName(char *name)
{
    sprintf(name, NAME(Banana) "_%d", linkaddr_node_addr.u8[0]);
}

int init()
{
    return 1;
}

#endif /* DATASET */
