#ifndef MAT_H
#define MAT_H

#undef __STRICT_ANSI__

#include "common.h"
#include <stdlib.h>
#include <math.h>

#define idx(i, j, n) (i*(n-1) - i*(i-1)/2 + j)

#pragma pack(1)

typedef float real;

typedef struct Mat {
    real arr[NUM_READINGS*(NUM_READINGS+1)/2];
} Mat;

typedef struct Vec {
    real arr[NUM_READINGS];
} Vec;

/*
 * norm() - calculates the norm of a vector.
 */
real norm(const real *v, size_t n)
{
    real sum = 0;
    for (int i = 0; i < n; i++)
        sum += v[i]*v[i];
    return sqrtf(sum);
}

/*
 * dot() - calculate the dot product of two vectors.
 */
real dot(const real *v1, const real *v2, size_t n)
{
    real sum = 0;
    for (int i = 0; i < n; i++)
        sum += v1[i]*v2[i];
    return sum;
}

/* Calculates the rbf kernel of two sensor data vectors of size n */
real rbf(const real *v1, const real *v2, size_t n)
{
    real diff[10]; // Enough space for vector from any dataset
    for (int i = 0; i < n; i++)
        diff[i] = v1[i] - v2[i];
    real nm = norm(diff, n);
    return expf(-nm*nm/(SIG*SIG));
}

/*
 * matvec() - right multiply a matrix by a vector.
 */
void matvec(const Mat *a, size_t m, size_t n, const Vec *x, Mat *out)
{
    for (int i = 0; i < m; i++) {
        real sum = 0;
        for (int j = 0; j < n; j++)
            sum += a->arr[idx(i, j, n)]*x->arr[j];
        out->arr[i] = sum;
    }
}

/*
 * matmul() - multiply two matrices of arbitrary size.
 */
void matmul(const Mat *a, const Mat *b, size_t m1, size_t n1, size_t n2, Mat *out)
{
    for (int i = 0; i < m1; i++) {
        for (int j = 0; j < n2; j++) {
            real sum = 0;
            for (int k = 0; k < n1; k ++)
                sum += a->arr[idx(i, k, n1)]*b->arr[idx(k, j, n2)];
            out->arr[idx(i, j, n2)] = sum;
        }
    }
}

/*
 * matadd() - adds one matrix to another.
 */
void matadd(const Mat *a, const Mat *b, size_t m, size_t n, Mat *out)
{
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            out->arr[idx(i, j, n)] = a->arr[idx(i, j, n)] + b->arr[idx(i, j, n)];
}

/*
 * matsub() - subtracts one matrix from another.
 */
void matsub(const Mat *a, const Mat *b, size_t m, size_t n, Mat *out)
{
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            out->arr[idx(i, j, n)] = a->arr[idx(i, j, n)] - b->arr[idx(i, j, n)];
}

/*
 * centMat() - centre the matrix k without using matrix multiplications.
 */
void centMat(Mat *k, size_t n)
{
    real means[NUM_READINGS] = {0};
    real mean = 0;
    for (int i = 0; i < n; i++) {
        means[i] += k->arr[idx(i, i, NUM_READINGS)];
        mean += k->arr[idx(i, i, NUM_READINGS)];
        for (int j = i + 1; j < n; j++) {
            means[i] += k->arr[idx(i, j, NUM_READINGS)];
            means[j] += k->arr[idx(i, j, NUM_READINGS)];
            mean += 2*k->arr[idx(i, j, NUM_READINGS)];
        }
    }
    for (int i = 0; i < n; i++)
        means[i] /= (real)n;
    mean /= (real)(n*n);
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            k->arr[idx(i, j, NUM_READINGS)] = k->arr[idx(i, j, NUM_READINGS)] - means[i] - means[j] + mean;
        }
    }
}

#endif /* MAT_H */
