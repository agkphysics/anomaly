#ifndef COMMON_H
#define COMMON_H

/* Required for the xxxf() functions */
#undef __STRICT_ANSI__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BUF_LEN 40
#define NUM_READINGS 20
#define NU 0.1

typedef float real;

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

/* RBF width parameter */
#define SIG 1

#endif /* COMMON_H */
