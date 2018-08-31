#ifndef COMMON_H
#define COMMON_H

/* Required for the xxxf() functions */
#undef __STRICT_ANSI__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BUF_LEN 60
#define NUM_READINGS 35
#define NU 0.3

/* RBF width parameter */
#define SIG 5.0

#ifdef NATIVE
    #define NAME(name) "data/" #name
#else
    #define NAME(name) #name
#endif

typedef float real;

static const char *delim = " \n";
const real scaling[VAL_LEN];

struct sensorval {
    uint16_t epoch;
    real vars[VAL_LEN];
};
typedef struct sensorval sensorval;

char * getNextReading(char *buf, struct sensorval *val)
{
    char *p;
    val->epoch = atoi(strtok(buf, delim));
    for (int i = 0; i < VAL_LEN; i++)
        val->vars[i] = (float)atol(p = strtok(NULL, delim)) / scaling[i];
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

/*
 * sort() - sort in ascending order using insertion sort
 */
void sort(real *list, size_t n)
{
    for (int i = 1; i < n; i++)
        for (int j = i; j > 0; j--)
            if (list[j] < list[j-1]) {
                real tmp = list[j-1];
                list[j-1] = list[j];
                list[j] = tmp;
            }
}

#endif /* COMMON_H */
