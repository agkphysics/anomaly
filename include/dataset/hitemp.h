#ifndef DATASET
#define DATASET

#include "contiki.h"

#define VAL_LEN 1
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>

#define COMMUNICATION
#define ROOT_NODE 1
#define NUM_SENSORS 10

const real scaling[VAL_LEN] = {1000};

void getName(char *name)
{
    sprintf(name, NAME(HITEMP) "_%d", linkaddr_node_addr.u8[0]);
}

int init()
{
    return 1;
}

#endif /* DATASET */
