#ifndef DATASET
#define DATASET

#include "contiki.h"
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>

#define VAL_LEN 4
#define COMMUNICATION
#define ROOT_NODE 1
#define NUM_SENSORS 52

struct sensorval {
    uint16_t epoch;
    real temp;
    real humidity;
    real lux;
    real voltage;
};
typedef struct sensorval sensorval;

static const char *delim = " \n";
char * getNextReading(char *buf, struct sensorval *val)
{
    char *p;
    val->epoch = atoi(strtok(buf, delim));
    val->temp = (float)atol(strtok(NULL, delim)) / 10000.0;
    val->humidity = (float)atol(strtok(NULL, delim)) / 10000.0;
    val->lux = (float)atol(strtok(NULL, delim)) / 100.0;
    val->voltage = (float)atol(p = strtok(NULL, delim)) / 100000.0;
    return p;
}

void getVector(const struct sensorval *val, float *vect)
{
    vect[0] = val->temp;
    vect[1] = val->humidity;
    vect[2] = val->lux;
    vect[3] = val->voltage;
}

void printReading(const struct sensorval *val)
{
    printf("Scaled sensor value for %d: (%ld, %ld, %ld, %ld)\n",
        val->epoch, (long)(val->temp*10000.0), (long)(val->humidity*10000.0),
        (long)(val->lux*100.0), (long)(val->voltage*100000.0));
}

void getName(char *name)
{
#ifdef NATIVE
    sprintf(name, "data/IBRL_%d", linkaddr_node_addr.u8[0]);
#else
    sprintf(name, "IBRL_%d", linkaddr_node_addr.u8[0]);
#endif
}

int init()
{
    /* Special nodes that are excluded due to invalid data */
    return !(linkaddr_node_addr.u8[0] == 5 || linkaddr_node_addr.u8[0] == 28);
}

#endif /* DATASET */
