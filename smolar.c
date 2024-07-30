#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "smolar.h"

/*
free all the memory allocated by an Array
*/
void smCleanup(Array *arr)
{
    free(arr->data);
    free(arr->shape);
    free(arr->strides);
    free(arr->backstrides);
    // clean up indices
    for (int i = 0; i < arr->idxs->count; i++)
    {
        free(arr->idxs->indices[i]);
    }
    free(arr->idxs->indices);
    free(arr->idxs);
    free(arr);
}

// ------------------- Utility functions --------------------

/*
takes in a float* or int* and checks if that is null.
*/
void _checkNull(void *value)
{
    if (value == NULL)
    {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }
}

/*
generate pseudo-random float or int

note: rand() is a pseudo random generator but for this implementation
we do not need a true random number generator. rand() introduces "modulo bias"
and the distribution of the random numbers generated won't be entirely uniform

more here: https://stackoverflow.com/questions/10984974/why-do-people-say-there-is-modulo-bias-when-using-a-random-number-generator
*/
float _getrandomFloat(float min, float max)
{
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

int _getRandomInt(int min, int max)
{
    return min + rand() % (max + 1 - min);
}

// ----------------- Required Array functions ------------------

/*
set C-order flag for an array
*/
void __checkOrderC__(Array *arr)
{
    bool val = (arr->strides[arr->ndim - 1] == arr->itemsize) ? true : false;
    arr->C_ORDER = val;
}

/*
set F-order flag for an array
*/
void __checkOrderF__(Array *arr)
{
    bool val = (arr->strides[0] == arr->itemsize) ? true : false;
    arr->F_ORDER = val;
}

/*
set flags for array
*/
void __setArrayFlags__(Array *arr)
{
    __checkOrderC__(arr);
    __checkOrderF__(arr);
}

/*
(re)calculate strides given a new shape
*/
void __recalculateStrides__(Array *arr)
{
    arr->strides[arr->ndim - 1] = arr->itemsize;
    for (int i = arr->ndim - 2; i >= 0; i--)
    {
        arr->strides[i] = arr->strides[i + 1] * arr->shape[i + 1];
    }
}

/*
(re)calculate backstrides for an Array
*/
void __recalculateBackstrides__(Array *arr)
{
    for (int i = arr->ndim - 1; i >= 0; i--)
    {
        arr->backstrides[i] = -1 * arr->strides[i] * (arr->shape[i] - 1);
    }
}

ArrayIndices *__getArrayIndicesFromShape__(const int *shape, int ndim)
{
    ArrayIndices *idxs = (ArrayIndices *)malloc(sizeof(ArrayIndices));
    _checkNull(idxs);

    int totalsize = 1;
    for (int i = 0; i < ndim; i++)
    {
        totalsize *= shape[i];
    }
    idxs->count = totalsize;
    idxs->indices = (int **)malloc(idxs->count * sizeof(int *));
    _checkNull(idxs->indices);

    for (int i = 0; i < idxs->count; i++)
    {
        idxs->indices[i] = (int *)malloc(ndim * sizeof(int));
        _checkNull(idxs->indices[i]);
    }
    int *current_index = (int *)calloc(ndim, sizeof(int));
    for (int i = 0; i < idxs->count; i++)
    {
        for (int j = 0; j < ndim; j++)
        {
            idxs->indices[i][j] = current_index[j];
        }
        for (int j = ndim - 1; j >= 0; j--)
        {
            if (++current_index[j] < shape[j])
            {
                break;
            }
            current_index[j] = 0;
        }
    }
    free(current_index);

    return idxs;
}

/*
function to create the arrayIndices
*/
void __createArrayIndices__(Array *arr)
{
    if (arr->idxs != NULL)
    {
        // clean up indices
        for (int i = 0; i < arr->idxs->count; i++)
        {
            free(arr->idxs->indices[i]);
        }
        free(arr->idxs->indices);
        free(arr->idxs);
    }

    arr->idxs = (ArrayIndices *)malloc(sizeof(ArrayIndices));
    _checkNull(arr->idxs);
    arr->idxs->count = arr->totalsize;
    arr->idxs->indices = (int **)malloc(arr->idxs->count * sizeof(int *));
    _checkNull(arr->idxs->indices);

    for (int i = 0; i < arr->idxs->count; i++)
    {
        arr->idxs->indices[i] = (int *)malloc(arr->ndim * sizeof(int));
        _checkNull(arr->idxs->indices[i]);
    }

    // generate all possible index combinations
    int *current_index = (int *)calloc(arr->ndim, sizeof(int));
    for (int i = 0; i < arr->idxs->count; i++)
    {
        for (int j = 0; j < arr->ndim; j++)
        {
            arr->idxs->indices[i][j] = current_index[j];
        }

        // increment the index
        for (int j = arr->ndim - 1; j >= 0; j--)
        {
            if (++current_index[j] < arr->shape[j])
            {
                break;
            }
            current_index[j] = 0;
        }
    }

    free(current_index);
}

/*
1D equivalent of the n-dimensional indices.
*/
void __createLinearIndices__(Array *arr)
{
    arr->lidxs = (LinearIndices *)malloc(sizeof(LinearIndices));
    _checkNull(arr->lidxs);
    arr->lidxs->count = arr->totalsize;
    arr->lidxs->indices = (int *)malloc(arr->lidxs->count * sizeof(int));
    _checkNull(arr->lidxs->indices);

    for (int i = 0; i < arr->totalsize; i++)
    {
        arr->lidxs->indices[i] = 0;
        for (int j = 0; j < arr->ndim; j++)
        {
            arr->lidxs->indices[i] += (arr->idxs->indices[i][j] * arr->strides[j]) / arr->itemsize;
        }
    }
}

// helper function to print ArrayIndices (for debugging)
void printArrayIndices(Array *arr)
{
    for (int i = 0; i < arr->idxs->count; i++)
    {
        printf("{");
        for (int j = 0; j < arr->ndim; j++)
        {
            printf("%d", arr->idxs->indices[i][j]);
            if (j < arr->ndim - 1)
            {
                printf(", ");
            }
        }
        printf("}\n");
    }
}

void __setArrayMetadata__(Array *arr) {
    __recalculateStrides__(arr);
    __recalculateBackstrides__(arr);
    __createArrayIndices__(arr);
    __createLinearIndices__(arr);
    __setArrayFlags__(arr);
}

/*
assume that the shape and number of dims are given,
create a new Array from that.
*/
Array *smCreate(const int *shape, int ndim)
{
    if (ndim <= 0)
    {
        fprintf(stderr, "Cannot initialize Array of dimensions %d", ndim);
        exit(1);
    }

    Array *arr = (Array *)malloc(sizeof(Array));
    _checkNull(arr);

    arr->ndim = ndim;
    arr->shape = (int *)malloc(arr->ndim * sizeof(int));
    arr->strides = (int *)malloc(arr->ndim * sizeof(int));
    arr->backstrides = (int *)malloc(arr->ndim * sizeof(int));

    _checkNull(arr->shape);
    _checkNull(arr->strides);
    _checkNull(arr->backstrides);

    arr->itemsize = sizeof(float);
    arr->totalsize = 1;

    for (int i = 0; i < arr->ndim; i++)
    {
        arr->shape[i] = shape[i];
        arr->totalsize *= shape[i];
    }

    __setArrayMetadata__(arr);

    // allocate data
    arr->data = (float *)malloc(arr->totalsize * arr->itemsize);
    _checkNull(arr->data);

    return arr;
}

bool __checkShapeCompatible__(Array *arr, const int *shape, int ndim)
{
    int size_new = 1;
    for (int i = 0; i < ndim; i++)
    {
        size_new *= shape[i];
    }

    return (arr->totalsize == size_new);
}

/*
initialize the Array's data with values (has to be 1D in memory)
assume values length the same as Array's totalsize
*/
void smFromValues(Array *arr, float *values)
{
    for (int i = 0; i < arr->totalsize; i++)
    {
        arr->data[i] = values[i];
    }
}

/*
random array from shape, values will be in range `[0.0, 1.0]`
*/
Array *smRandom(const int *shape, int ndim)
{
    Array *arr = smCreate(shape, ndim);

    for (int i = 0; i < arr->totalsize; i++)
    {
        arr->data[i] = _getrandomFloat(0.f, 1.f);
    }

    return arr;
}

/*
similar to numpy's arange
*/
Array *smArange(float start, float end, float step)
{
    if (start >= end)
    {
        fprintf(stderr, "Start value should not be greater than or equal to end value.\n");
        exit(1);
    }
    if (step <= 0)
    {
        fprintf(stderr, "Step value should not be less than or equal to 0.\n");
        exit(1);
    }

    // length of result array
    int _len = 0;
    float curr = start;
    while (curr < end)
    {
        curr += step;
        _len++;
    }

    int res_shape[] = {_len};
    int ndim = 1;

    Array *res = smCreate(res_shape, ndim);
    curr = start;
    for (int i = 0; i < res->totalsize; i++)
    {
        res->data[i] = curr;
        curr += step;
    }

    return res;
}

/*
show the shape/strides/backstrides of an Array
as a python tuple for each dimension: `(n, m, k, ...)`
*/
void __printArrayInternals__(Array *arr, int *s)
{
    for (int i = 0; i < arr->ndim; i++)
    {
        if (i == 0)
            fprintf(stdout, "( ");

        if (i == arr->ndim - 1)
            fprintf(stdout, "%d )", s[i]);
        else
            fprintf(stdout, "%d, ", s[i]);
    }
    fprintf(stdout, "\n");
}

// Array's data as it is laid out in memory
void __printArrayData__(Array *arr)
{
    for (int i = 0; i < arr->totalsize; i++)
    {
        fprintf(stdout, "%.3f ", arr->data[i]);
    }

    fprintf(stdout, "\n");
}
/*
prints the array info: shape, strides, order of Array (C or F)
*/
void smPrintInfo(Array *arr)
{
    fprintf(stdout, "Shape: ");
    __printArrayInternals__(arr, arr->shape);
    fprintf(stdout, "Strides: ");
    __printArrayInternals__(arr, arr->strides);
    fprintf(stdout, "Array is C-contiguous? %s\n", arr->C_ORDER ? "true" : "false");
    fprintf(stdout, "Array is F-contiguous? %s\n", arr->F_ORDER ? "true" : "false");
}

// recursive helper
char *__traverseHelper__(
    char *curr, int *shape, int *strides, int *backstrides,
    int ndim, int depth)
{
    // we are at the last dimension
    if (depth == ndim - 1)
    {
        fprintf(stdout, "%.3f ", *(float *)curr);
        for (int i = 0; i < shape[ndim - 1] - 1; i++)
        {
            curr += strides[ndim - 1];
            fprintf(stdout, "%.3f ", *(float *)curr);
        }

        // backstep
        curr += backstrides[ndim - 1];
        printf("\n");
        return curr;
    }

    curr = __traverseHelper__(curr, shape, strides, backstrides, ndim, depth + 1);
    for (int i = 0; i < shape[depth] - 1; i++)
    {
        curr += strides[depth];
        curr = __traverseHelper__(curr, shape, strides, backstrides, ndim, depth + 1);
    }

    curr += backstrides[depth];

    if (depth != 0)
        printf("\n");

    return curr;
}

/*
traverse array according to backstrides and execute a printf on each element
*/
void smShow(Array *arr)
{
    char *curr = (char *)arr->data;

    curr = __traverseHelper__(
        curr, arr->shape, arr->strides, arr->backstrides,
        arr->ndim, 0);
}

// ------------------- Operations with Arrays -------------------

/*
- ndim should be equal
- each element in the shape should be equal
*/
int smCheckShapesEqual(Array *a, Array *b)
{
    int equal = 1;

    if (a->ndim != b->ndim)
        equal = 0;

    else
    {
        for (int i = 0; i < a->ndim; i++)
            if (a->shape[i] != b->shape[i])
            {
                equal = 0;
                break;
            }
    }

    return equal;
}

/*
this function essentially finds the final shape after
performing broadcasting (if needed) between any two Arrays.

returns `NULL` if shapes aren't broadcastable
*/
int *__broadcastFinalShape__(Array *a, Array *b)
{
    if (smCheckShapesEqual(a, b))
        return a->shape;

    int res_ndim = (a->ndim > b->ndim) ? a->ndim : b->ndim;

    // left's 1s to prepend
    int l1add = res_ndim - a->ndim;
    int r1add = res_ndim - b->ndim;

    int lf_shape[res_ndim], rf_shape[res_ndim];
    int *res_shape = (int *)malloc(res_ndim * sizeof(int));

    // a
    int inda = 0;
    for (int i = 0; i < res_ndim; i++)
    {
        if (i < l1add)
            lf_shape[i] = 1;
        else
        {
            lf_shape[i] = a->shape[inda];
            inda++;
        }
    }

    // b
    int indb = 0;
    for (int i = 0; i < res_ndim; i++)
    {
        if (i < r1add)
            rf_shape[i] = 1;
        else
        {
            rf_shape[i] = b->shape[indb];
            indb++;
        }
    }

    for (int i = 0; i < res_ndim; i++)
    {
        if (lf_shape[i] == 1 || rf_shape[i] == 1 || lf_shape[i] == rf_shape[i])
            res_shape[i] = (lf_shape[i] > rf_shape[i]) ? lf_shape[i] : rf_shape[i];
        else
            return NULL;
    }

    return res_shape;
}

/*
broadcasts an array given a new shape and its dimensions.

note: errors are not checked, and it is to be assumed that the Array
is "broadcastable" to the new shape.

if you use this function, you will have to manually free the result of broadcasted array
*/
Array *__broadcastArray__(Array *arr, const int *shape, int ndim)
{
    Array *res = smCreate(shape, ndim);

    int n_prepend = ndim - arr->ndim;

    for (int i = 0; i < res->totalsize; i++)
    {
        int src_idx = 0;
        for (int dim = 0; dim < arr->ndim; dim++)
        {
            if (arr->shape[dim] > 1)
            {
                src_idx += (
                    res->idxs->indices[i][n_prepend + dim] % arr->shape[dim]
                ) * arr->strides[dim];
            }
        }

        // copy the value from source index to res
        res->data[res->lidxs->indices[i]] = arr->data[src_idx / arr->itemsize];
    }
    return res;
}

/*
reshape an Array to new shape and new ndim
*/
Array *smReshapeNew(Array *arr, const int *shape, int ndim)
{
    bool possible = __checkShapeCompatible__(arr, shape, ndim);
    if (!possible)
    {
        fprintf(stderr, "Cannot reshape due to invalid shape.\n");
        exit(1);
    }

    Array *res = smCreate(shape, ndim);
    smFromValues(res, arr->data);

    return res;
}

/*
inplace reshape operation

this will just change the shape and strides of the Array.
the underlying data in the memory is not touched.
*/
void smReshapeInplace(Array *arr, const int *shape, int ndim)
{
    bool possible = __checkShapeCompatible__(arr, shape, ndim);
    if (!possible)
    {
        fprintf(stderr, "Cannot reshape due to invalid shape.\n");
        exit(1);
    }

    arr->ndim = ndim;
    for (int i = 0; i < ndim; i++)
    {
        arr->shape[i] = shape[i];
    }

    __setArrayMetadata__(arr);
}

/*
transpose an Array along given permutation of axes
assume axes is a valid permutation
*/
Array *smTransposeNew(Array *arr, const int *axes)
{
    Array *res = smCreate(arr->shape, arr->ndim);
    if (arr->ndim == 1)
    {
        smFromValues(res, arr->data);
        return res;
    }

    // set data
    smFromValues(res, arr->data);

    int *_axes = (int *)malloc(res->ndim * sizeof(int));
    _checkNull(_axes);

    if (axes == NULL)
    {
        for (int i = 0; i < res->ndim; i++)
        {
            _axes[i] = res->ndim - 1 - i;
        }
    }

    int *newshape = (int *)malloc(res->ndim * sizeof(int));
    int *newstrides = (int *)malloc(res->ndim * sizeof(int));
    _checkNull(newshape);
    _checkNull(newstrides);

    for (int i = 0; i < res->ndim; i++)
    {
        newshape[i] = (axes != NULL) ? arr->shape[axes[i]] : arr->shape[_axes[i]];
        newstrides[i] = (axes != NULL) ? arr->strides[axes[i]] : arr->strides[_axes[i]];
    }
    for (int i = 0; i < res->ndim; i++)
    {
        res->shape[i] = newshape[i];
        res->strides[i] = newstrides[i];
    }
    free(newshape);
    free(newstrides);
    free(_axes);

    __recalculateBackstrides__(res);
    __createArrayIndices__(res);
    __createLinearIndices__(res);
    __setArrayFlags__(res);

    return res;
}

/*
can be parallelized.
*/
Array *__PaddArrays__(Array *a, Array *b)
{
    Array *res = smCreate(a->shape, a->ndim);

    for (int i = 0; i < a->totalsize; i++)
    {
        res->data[i] = a->data[i] + b->data[i];
    }

    return res;
}

Array *__PmulArrays__(Array *a, Array *b)
{
    Array *res = smCreate(a->shape, a->ndim);

    for (int i = 0; i < a->totalsize; i++)
    {
        res->data[i] = a->data[i] * b->data[i];
    }

    return res;
}

/*
add the elements of two Arrays elementwise
if the shapes are not equal but broadcastable,
then broadcasting will take place.
*/
Array *smAdd(Array *a, Array *b)
{
    if (smCheckShapesEqual(a, b))
        return __PaddArrays__(a, b);

    int *res_shape = __broadcastFinalShape__(a, b);

    if (res_shape == NULL)
    {
        fprintf(stderr, "Cannot add Arrays of non-broadcastable shapes.\n");
        exit(1);
    }

    // broadcast shapes
    // free broadcasts
    int res_ndim = (a->ndim > b->ndim) ? a->ndim : b->ndim;

    Array *afinal = __broadcastArray__(a, res_shape, res_ndim);
    Array *bfinal = __broadcastArray__(b, res_shape, res_ndim);

    Array *res = __PaddArrays__(afinal, bfinal);

    smCleanup(afinal);
    smCleanup(bfinal);
    free(res_shape);

    return res;
}

/*
-1 * arr->data
*/
Array *__PnegArray__(Array *arr)
{
    Array *res = smCreate(arr->shape, arr->ndim);

    for (int i = 0; i < res->totalsize; i++)
    {
        res->data[i] = -1 * arr->data[i];
    }

    return res;
}

/*
this is basically a + (-1 * b) elementwise
*/
Array *__PsubArrays__(Array *a, Array *b)
{
    Array *bneg = __PnegArray__(b);

    Array *res = smAdd(a, bneg);

    smCleanup(bneg);
    return res;
}

/*
multiply the elements of two Arrays elementwise
for now, assume shapes of both are same

TODO: implement broadcasting
*/
Array *smMul(Array *a, Array *b)
{
    if (smCheckShapesEqual(a, b))
        return __PmulArrays__(a, b);

    int *res_shape = __broadcastFinalShape__(a, b);

    if (res_shape == NULL)
    {
        fprintf(stderr, "Cannot multiply Arrays of non-broadcastable shapes.\n");
        exit(1);
    }

    // broadcast shapes
    // free broadcasts
    int res_ndim = (a->ndim > b->ndim) ? a->ndim : b->ndim;

    Array *afinal = __broadcastArray__(a, res_shape, res_ndim);
    Array *bfinal = __broadcastArray__(b, res_shape, res_ndim);

    Array *res = __PmulArrays__(afinal, bfinal);

    smCleanup(afinal);
    smCleanup(bfinal);
    free(res_shape);

    return res;
}

/*
Expand any axis in an array.
For example: If arr->shape is (3,) then
smExpandAxis(arr, 0)->shape will be (1, 3)
smExpandAxis(arr, 1)->shape will be (3, 1) and so on...
*/
Array *smExpandDims(Array *arr, int axis)
{
    // handle negative values
    if (axis < 0)
        axis = arr->ndim + axis + 1;

    if (axis < 0 || axis > arr->ndim)
    {
        fprintf(stderr, ">> error: axis out of bounds for expanding.\n");
        exit(1);
    }

    int new_ndim = arr->ndim + 1;
    int *new_shape = (int *)malloc(new_ndim * sizeof(int));
    _checkNull(new_shape);

    // copy into new shape
    int j = 0;
    for (int i = 0; i < new_ndim; i++)
    {
        if (i == axis)
            new_shape[i] = 1;
        else
        {
            new_shape[i] = arr->shape[j];
            j++;
        }
    }

    // copy data
    Array *result = smCreate(new_shape, new_ndim);
    memcpy(result->data, arr->data, arr->totalsize * sizeof(float));

    free(new_shape);
    return result;
}

/*
Squeeze any axis in an array.
For example: If arr->shape is (3, 1) then
smExpandAxis(arr, 1)->shape will be (3, ) and so on...
*/
Array *smSqueeze(Array *arr, int axis)
{
    if (axis < 0)
        axis = arr->ndim + axis;

    if (axis < 0 || axis > arr->ndim)
    {
        fprintf(stderr, ">> error: axis out of bounds for expanding.\n");
        exit(1);
    }
    if (arr->ndim == 1)
    {
        fprintf(stderr, ">> error: cannot squeeze Array with ndim 1.\n");
        exit(1);
    }

    printf("axis is %d\n", axis);
    int new_ndim = arr->ndim - 1;
    int *new_shape = (int *)malloc(new_ndim * sizeof(int));
    _checkNull(new_shape);

    // copy into new shape
    int j = 0;
    for (int i = 0; i < arr->ndim; i++)
    {
        if (i == axis && arr->shape[i] != 1)
        {
            fprintf(stderr, ">> error: dimension of the axis to squeeze must be 1.\n");
            exit(1);
        }
        else if (i == axis && arr->shape[i] == 1)
            continue;
        else
        {
            new_shape[j] = arr->shape[i];
            j++;
        }
    }

    // copy data
    Array *result = smCreate(new_shape, new_ndim);
    memcpy(result->data, arr->data, arr->totalsize * sizeof(float));

    free(new_shape);
    return result;
}

/*
Dot product between two vectors i.e. Arrays with dimension 1.
Returns: Array with shape {1} i.e. only one element.
*/
Array *smDot(Array *a, Array *b)
{
    if (a->ndim != 1 && b->ndim != 1)
    {
        fprintf(stderr, ">> error: both arrays should be vectors for dot product.");
        exit(1);
    }
    if (a->shape[0] != b->shape[0])
    {
        fprintf(stderr, ">> error: both vectors should have the same shape for dot product.");
        exit(1);
    }

    float dot = 0.0f;
    int shape[] = {1};
    Array *result = smCreate(shape, 1);

    for (int i = 0; i < a->totalsize; i++)
    {
        dot += (a->data[i] * b->data[i]);
    }
    result->data[0] = dot;

    return result;
}

/*
matrix multiplication of n-dimensional arrays.
```
for 2D arrays (m, n) @ (n, d) = (m, d)
for nD arrays (..., m, n) @ (..., n, d) = (..., m, d)
```
when the dimensions of arrays are greater than 2, we do N matmuls
on the last two axes of the operands. These N matmuls will be stacked
in the shape of the higher dimensions.
*/
Array *smMatMul(Array *a, Array *b)
{
    if (a->ndim < 2 || b->ndim < 2)
    {
        fprintf(stderr, ">> Error: both arrays must have at least 2 dimensions for matmul.\n");
        return NULL;
    }
    if (a->shape[a->ndim - 1] != b->shape[b->ndim - 2])
    {
        fprintf(stderr, ">> Error: last dimension of first array must match second-last dimension of second array.\n");
        return NULL;
    }

    int result_ndim = (a->ndim > b->ndim) ? a->ndim : b->ndim;
    int *result_shape = (int *)malloc(result_ndim * sizeof(int));

    // broadcast result shape untill last two axes
    for (int i = 0; i < result_ndim - 2; i++)
    {
        result_shape[i] = (i < a->ndim - 2) ? a->shape[i] : 1;
        result_shape[i] = (i < b->ndim - 2) ? (result_shape[i] > b->shape[i] ? result_shape[i] : b->shape[i]) : result_shape[i];
    }
    result_shape[result_ndim - 2] = a->shape[a->ndim - 2];
    result_shape[result_ndim - 1] = b->shape[b->ndim - 1];

    Array *result = smCreate(result_shape, result_ndim);
    free(result_shape);

    int m = a->shape[a->ndim - 2];
    int n = a->shape[a->ndim - 1];
    int p = b->shape[b->ndim - 1];

    // total matmuls to perform
    int last_two = result->shape[result_ndim - 1] * result->shape[result_ndim - 2];
    int totalops = result->totalsize / last_two;

    ArrayIndices *idxs = __getArrayIndicesFromShape__(result->shape, result_ndim - 2);

    for (int idx = 0; idx < totalops; idx++)
    {
        int *nd_index = idxs->indices[idx];

        // perform matmul for this slice
        // note: can be parallelized
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < p; j++)
            {
                float sum = 0.0f;
                for (int k = 0; k < n; k++)
                {
                    // linear 1D index for a and b
                    int a_index1d = 0, b_index1d = 0;

                    // higher dimensions
                    for (int d = 0; d < a->ndim - 2; d++)
                        a_index1d += (nd_index[d] * a->strides[d]);
                    for (int d = 0; d < b->ndim - 2; d++)
                        b_index1d += (nd_index[d] * b->strides[d]);

                    // last 2 dimensions
                    a_index1d += (i * a->strides[a->ndim - 2] + k * a->strides[a->ndim - 1]);
                    b_index1d += (k * b->strides[b->ndim - 2] + j * b->strides[b->ndim - 1]);

                    sum += a->data[a_index1d / a->itemsize] * b->data[b_index1d / b->itemsize];
                }

                // same as a and b, for result
                int r_index1d = 0;
                for (int d = 0; d < a->ndim - 2; d++)
                    r_index1d += (nd_index[d] * result->strides[d]);
                r_index1d += (i * result->strides[result->ndim - 2] + j * result->strides[result->ndim - 1]);

                result->data[r_index1d / result->itemsize] = sum;
            }
        }
    }
    free(idxs);

    return result;
}

/*
Apply a given ArrayFunc element-wise to the array, inplace
*/
void smApplyInplace(Array *arr, ArrayFunc func)
{
    for (int i = 0; i < arr->totalsize; i++)
    {
        arr->data[i] = func(arr->data[i]);
    }
}

// --------------------------------------------------------------

float square(float x)
{
    return x * x;
}

float cube(float x)
{
    return x * x * x;
}
