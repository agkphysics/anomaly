#ifndef DATASET
#define DATASET

#include "contiki.h"

#define VAL_LEN 4
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>

#define COMMUNICATION
#define ROOT_NODE 1
#define NUM_SENSORS 52

const real scaling[VAL_LEN] = {10000, 10000, 100, 100000};

void getName(char *name)
{
    sprintf(name, NAME(IBRL) "_%d", linkaddr_node_addr.u8[0]);
}

int init()
{
    /* Special nodes that are excluded due to invalid data */
    return !(linkaddr_node_addr.u8[0] == 5 || linkaddr_node_addr.u8[0] == 28);
}

#endif /* DATASET */
