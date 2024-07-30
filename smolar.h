/*
smolar - a tiny multidimensional array library in just one file

well, it is obvious this file will contain A LOT but there are some naming
conventions that i follow here (i try):

Array:
* "public" functions or functions to be used start from "sm" such as: smAdd, smRandom
* "private" array functions start and end with double underscores "__"
* functions that can be parallelized will start from "P" or "__P"
* function names can be long dw

Utility:
* these functions start from a single underscore "_"

every function uses `camelCase` for its name, and that's it.

one thing to note here is every Array has only `float` dtype
*/

#ifndef SMOLAR_H
#define SMOLAR_H

#include <stdbool.h>

typedef float (*ArrayFunc)(float);

typedef struct
{
    int **indices; // 2D array to hold all possible index combinations
    int count;     // total number of index combinations
} ArrayIndices;

typedef struct
{
    int *indices;
    int count;
} LinearIndices;

typedef struct
{
    float *data;      // holds the actual data in continuous way
    int *shape;       // shape of the array
    int *strides;     // number of bytes to skip for each dimension
    int *backstrides; // reverse of strides; how many bytes to skip to go reverse

    int ndim;      // number of dimensions
    int itemsize;  // size of one element in the array
    int totalsize; // total size to allocate

    ArrayIndices *idxs;   // n-dimensional indices
    LinearIndices *lidxs; // 1D index

    bool C_ORDER;
    bool F_ORDER;
} Array;

// private
void __checkOrderC__(Array *arr);
void __checkOrderF__(Array *arr);
void __setArrayFlags__(Array *arr);
void __recalculateStrides__(Array *arr);
void __recalculateBackstrides__(Array *arr);
ArrayIndices *__getArrayIndicesFromShape__(const int *shape, int ndim);
void __createArrayIndices__(Array *arr);
void __createLinearIndices__(Array *arr);
bool __checkShapeCompatible__(Array *arr, const int *shape, int ndim);
void __printArrayInternals__(Array *arr, int *s);
void __printArrayData__(Array *arr);
void __setArrayMetadata__(Array *arr);
int *__broadcastFinalShape__(Array *a, Array *b);
Array *__broadcastArray__(Array *arr, const int *shape, int ndim);

// creation and management
Array *smCreate(const int *shape, int ndim);
void smCleanup(Array *arr);
Array *smRandom(const int *shape, int ndim);
Array *smArange(float start, float end, float step);
void smFromValues(Array *arr, float *values);

// information and display
void smPrintInfo(Array *arr);
void smShow(Array *arr);

// operations
int smCheckShapesEqual(Array *a, Array *b);
Array *smReshapeNew(Array *arr, const int *shape, int ndim);
void smReshapeInplace(Array *arr, const int *shape, int ndim);
Array *smTransposeNew(Array *arr, const int *axes);
Array *smAdd(Array *a, Array *b);
Array *smMul(Array *a, Array *b);
Array *smExpandDims(Array *arr, int axis);
Array *smSqueeze(Array *arr, int axis);
Array *smDot(Array *a, Array *b);
Array *smMatMul(Array *a, Array *b);
void smApplyInplace(Array *arr, ArrayFunc func);

// utility functions
float _getrandomFloat(float min, float max);
int _getRandomInt(int min, int max);

#endif // SMOLAR_H