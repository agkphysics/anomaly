#ifndef HIWS_H
#define HIWS_H

#include "contiki.h"
#include "common.h"

#include <stdlib.h>
#include <stdio.h>

#define VAL_LEN 6

struct HIWS {
    uint16_t epoch;
    real AIRT;
    real ATMP;
    real RAIN_AMOUNT;
    real RELH;
    real WDIR;
    real WSPD;
};
typedef struct HIWS sensorval;

static const char *delim = " \n";
char * getNextReading(char *buf, struct HIWS *val)
{
    char *p;
    val->epoch = atoi(strtok(buf, delim));
    val->AIRT = (float)atol(strtok(NULL, delim)) / 10.0;
    val->ATMP = (float)atol(strtok(NULL, delim)) / 10.0;
    val->RAIN_AMOUNT = (float)atol(strtok(NULL, delim)) / 10.0;
    val->RELH = (float)atol(strtok(NULL, delim)) / 10.0;
    val->WDIR = (float)atol(strtok(NULL, delim)) / 100.0;
    val->WSPD = (float)atol(p = strtok(NULL, delim)) / 100.0;
    return p;
}

void getVector(const struct HIWS *val, float *vect)
{
    vect[0] = val->AIRT;
    vect[1] = val->ATMP;
    vect[2] = val->RAIN_AMOUNT;
    vect[3] = val->RELH;
    vect[4] = val->WDIR;
    vect[5] = val->WSPD;
}

void printReading(const struct HIWS *val)
{
    printf("Scaled sensor value for %d: (%ld, %ld, %ld, %ld, %ld, %ld)\n",
        val->epoch, (long)(val->AIRT*10.0), (long)(val->ATMP*10.0),
        (long)(val->RAIN_AMOUNT*10.0), (long)(val->RELH*10.0),
        (long)(val->WDIR*100.0), (long)(val->WSPD*100.0));
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

#endif /* HIWS_H */
