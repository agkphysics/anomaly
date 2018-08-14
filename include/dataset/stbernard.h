#ifndef DATASET
#define DATASET

#include "contiki.h"
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>

#define VAL_LEN 9
#define COMMUNICATION
#define ROOT_NODE 1
#define NUM_SENSORS 5

struct sensorval {
    uint16_t epoch;
    real Ambient;
    real Surface;
    real Radiation;
    real Humidity;
    real SoilMoisture;
    real Watermark;
    real Rain;
    real WindSpeed;
    real WindDir;
};
typedef struct sensorval sensorval;

static const char *delim = " \n";
char * getNextReading(char *buf, struct sensorval *val)
{
    char *p;
    val->epoch = atoi(strtok(buf, delim));
    val->Ambient = (float)atol(strtok(NULL, delim)) / 1000.0;
    val->Surface = (float)atol(strtok(NULL, delim)) / 1000.0;
    val->Radiation = (float)atol(strtok(NULL, delim)) / 1000.0;
    val->Humidity = (float)atol(strtok(NULL, delim)) / 1000.0;
    val->SoilMoisture = (float)atol(strtok(NULL, delim)) / 1000.0;
    val->Watermark = (float)atol(strtok(NULL, delim)) / 1000.0;
    val->Rain = (float)atol(strtok(NULL, delim)) / 1000.0;
    val->WindSpeed = (float)atol(strtok(NULL, delim)) / 1000.0;
    val->WindDir = (float)atol(p =strtok(NULL, delim)) / 1000.0;
    return p;
}

void getVector(const struct sensorval *val, float *vect)
{
    vect[0] = val->Ambient;
    vect[1] = val->Surface;
    vect[2] = val->Radiation;
    vect[3] = val->Humidity;
    vect[4] = val->SoilMoisture;
    vect[5] = val->Watermark;
    vect[6] = val->Rain;
    vect[7] = val->WindSpeed;
    vect[8] = val->WindDir;
}

void printReading(const struct sensorval *val)
{

}

void getName(char *name)
{
#ifdef NATIVE
    sprintf(name, "data/StBernard");
#else
    sprintf(name, "StBernard");
#endif
}

int init()
{
    return 1;
}

#endif /* DATASET */
