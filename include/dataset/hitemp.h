#ifndef DATASET
#define DATASET

#include "contiki.h"
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>

#define VAL_LEN 1
#define COMMUNICATION
#define ROOT_NODE 1
#define NUM_SENSORS 10

struct sensorval {
    uint16_t epoch;
    real temp;
};
typedef struct sensorval sensorval;

static const char *delim = " \n";
char * getNextReading(char *buf, struct sensorval *val)
{
    char *p;
    val->epoch = atoi(strtok(buf, delim));
    val->temp = (float)atol(p = strtok(NULL, delim)) / 1000.0;
    return p;
}

void getVector(const struct sensorval *val, float *vect)
{
    vect[0] = val->temp;
}

void printReading(const struct sensorval *val)
{
    printf("Scaled sensor value for %d: (%ld)\n",
        val->epoch, (long)(val->temp*1000.0));
}

void getName(char *name)
{
#ifdef NATIVE
    sprintf(name, "data/HITEMP_%d", linkaddr_node_addr.u8[0]);
#else
    sprintf(name, "HITEMP_%d", linkaddr_node_addr.u8[0]);
#endif
}

int init()
{
    return 1;
}

#endif /* DATASET */