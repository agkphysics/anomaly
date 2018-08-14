#ifndef DATASET
#define DATASET

#include "contiki.h"
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>

#define VAL_LEN 6
#ifdef COMMUNICATION
#undef COMMUNICATION
#endif
#define ROOT_NODE 1
#define NUM_SENSORS 1

const real scaling[VAL_LEN] = {10, 10, 10, 10, 100, 100};

struct sensorval {
    uint16_t epoch;
    real vars[VAL_LEN];
    // real AIRT;
    // real ATMP;
    // real RAIN_AMOUNT;
    // real RELH;
    // real WDIR;
    // real WSPD;
};
typedef struct sensorval sensorval;

static const char *delim = " \n";
char * getNextReading(char *buf, struct sensorval *val)
{
    char *p;
    val->epoch = atoi(strtok(buf, delim));
    for (int i = 0; i < VAL_LEN; i++)
        val->vars[i] = (float)atol(p = strtok(NULL, delim)) / scaling[i];
    // val->AIRT = (float)atol(strtok(NULL, delim)) / 10.0;
    // val->ATMP = (float)atol(strtok(NULL, delim)) / 10.0;
    // val->RAIN_AMOUNT = (float)atol(strtok(NULL, delim)) / 10.0;
    // val->RELH = (float)atol(strtok(NULL, delim)) / 10.0;
    // val->WDIR = (float)atol(strtok(NULL, delim)) / 100.0;
    // val->WSPD = (float)atol(p = strtok(NULL, delim)) / 100.0;
    return p;
}

void getVector(const struct sensorval *val, float *vect)
{
    for (int i = 0; i < VAL_LEN; i++)
        vect[i] = val->vars[i];
}

void printReading(const struct sensorval *val)
{
    printf("Scaled sensor value for %d: (", val->epoch);
    for (int i = 0; i < VAL_LEN; i++)
        printf("%ld, ", (long)(val->vars[i]*scaling[i]));
    printf(")\n");
}

void getName(char *name)
{
#ifdef NATIVE
    sprintf(name, "data/HIWS");
#else
    sprintf(name, "HIWS");
#endif
}

int init()
{
    return 1;
}

#endif /* DATASET */
