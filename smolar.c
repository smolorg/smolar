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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <omp.h>

typedef float (*ArrayFunc)(float);
// our hero
typedef struct {
    float* data;        // holds the actual data in continuous way

    int* shape;         // shape of the array
    int* strides;       // number of bytes to skip for each dimension
    int* backstrides;   // reverse of strides; how many bytes to skip to go reverse

    int ndim;           // number of dimensions
    int itemsize;       // size of one element in the array
    int totalsize;      // total size to allocate

    bool C_ORDER;       // flag if array is c-order
    bool F_ORDER;       // flag if array is f-order
} Array;


/*
free all the memory allocated by an Array
*/
void smCleanup(Array* arr) {
    free(arr->data);
    free(arr->shape);
    free(arr->strides);
    free(arr->backstrides);
    free(arr);
}


// ------------------- Utility functions --------------------


/*
takes in a float* or int* and checks if that is null.
*/
void _checkNull(void* value) {
    if(value == NULL) {
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
float _getrandomFloat(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

int _getRandomInt(int min, int max) {
    return min + rand() % (max + 1 - min);
}


// ----------------- Required Array functions ------------------


/*
set C-order flag for an array
*/
void __checkOrderC__(Array* arr) {
    bool val = (arr->strides[arr->ndim - 1] == arr->itemsize) ? true: false;
    arr->C_ORDER = val;
}

/*
set F-order flag for an array
*/
void __checkOrderF__(Array* arr) {
    bool val = (arr->strides[0] == arr->itemsize) ? true: false;
    arr->F_ORDER = val;
}

/*
set flags for array
*/
void __setArrayFlags__(Array* arr) {
    __checkOrderC__(arr);
    __checkOrderF__(arr);
}

/*
(re)calculate strides given a new shape
*/
void __recalculateStrides__(Array* arr) {
    arr->strides[arr->ndim - 1] = arr->itemsize;
    for(int i = arr->ndim - 2; i >= 0; i--) {
        arr->strides[i] = arr->strides[i + 1] * arr->shape[i + 1];
    }
}

/*
(re)calculate backstrides for an Array
*/
void __recalculateBackstrides__(Array* arr) {
    for(int i = arr->ndim - 1; i >= 0; i--) {
        arr->backstrides[i] = -1 * arr->strides[i] * (arr->shape[i] - 1);
    }
}

/*
assume that the shape and number of dims are given,
create a new Array from that.
*/
Array* smCreate(const int* shape, int ndim) {
    if(ndim <= 0) {
        fprintf(stderr, "Cannot initialize Array of dimensions %d", ndim);
        exit(1);
    }

    Array* arr = (Array*)malloc(sizeof(Array));
    _checkNull(arr);

    arr->ndim = ndim;
    arr->shape = (int*)malloc(arr->ndim * sizeof(int));
    arr->strides = (int*)malloc(arr->ndim * sizeof(int));
    arr->backstrides = (int*)malloc(arr->ndim * sizeof(int));

    _checkNull(arr->shape); _checkNull(arr->strides); _checkNull(arr->backstrides);

    arr->itemsize = sizeof(float);
    arr->totalsize = 1;

    for(int i=0; i<arr->ndim; i++) {
        arr->shape[i] = shape[i];
        arr->totalsize *= shape[i];
    }

    // populate strides
    // by default C-order
    // so strides[ndim - 1] = itemsize
    __recalculateStrides__(arr);
    __recalculateBackstrides__(arr);
    __setArrayFlags__(arr);

    // allocate data
    arr->data = (float*)malloc(arr->totalsize * arr->itemsize);
    _checkNull(arr->data);

    return arr;
}

bool __checkShapeCompatible__(Array* arr, const int* shape, int ndim) {
    int size_new = 1;
    for(int i=0; i<ndim; i++) {
        size_new *= shape[i];
    }

    return (arr->totalsize == size_new);
}

/*
initialize the Array's data with values (has to be 1D in memory)
assume values length the same as Array's totalsize
*/
void smFromValues(Array* arr, float* values) {
    for(int i=0; i<arr->totalsize; i++) {
        arr->data[i] = values[i];
    }
}

/*
random array from shape, values will be in range `[0.0, 1.0]`
*/
Array* smRandom(const int* shape, int ndim) {
    Array* arr = smCreate(shape, ndim);

    #ifdef PARALLEL
    #pragma omp parallel for
    #endif
    for(int i=0; i<arr->totalsize; i++) {
        arr->data[i] = _getrandomFloat(0.f, 1.f);
    }

    return arr;
}

/*
similar to numpy's arange
*/
Array* smArange(float start, float end, float step) {
    if(start >= end) {
        fprintf(stderr, "Start value should not be greater than or equal to end value.\n");
        exit(1);
    }
    if(step <= 0) {
        fprintf(stderr, "Step value should not be less than or equal to 0.\n");
        exit(1);
    }

    // length of result array
    int _len = 0;
    float curr = start;
    while(curr < end) {
        curr += step;
        _len++;
    }

    int res_shape[] = {_len};
    int ndim = 1;

    Array* res = smCreate(res_shape, ndim);
    curr = start;
    for(int i=0; i<res->totalsize; i++) {
        res->data[i] = curr;
        curr += step;
    }

    return res;
}

/*
show the shape/strides/backstrides of an Array 
as a python tuple for each dimension: `(n, m, k, ...)`
*/
void __printArrayInternals__(Array* arr, int* s) {
    for(int i=0; i<arr->ndim; i++) {
        if(i == 0) fprintf(stdout, "( ");

        if(i == arr->ndim - 1)  fprintf(stdout, "%d )", s[i]);
        else    fprintf(stdout, "%d, ", s[i]);
    }
    fprintf(stdout, "\n");
}

// Array's data as it is laid out in memory
void __printArrayData__(Array* arr) {
    for(int i=0; i<arr->totalsize; i++) {
        fprintf(stdout, "%.3f ", arr->data[i]);
    }

    fprintf(stdout, "\n");
}
/*
prints the array info: shape, strides, order of Array (C or F)
*/
void smPrintInfo(Array* arr) {
    fprintf(stdout, "Shape: ");
    __printArrayInternals__(arr, arr->shape);
    fprintf(stdout, "Strides: ");
    __printArrayInternals__(arr, arr->strides);
    fprintf(stdout, "Array is C-contiguous? %s\n", arr->C_ORDER ? "true": "false");
    fprintf(stdout, "Array is F-contiguous? %s\n", arr->F_ORDER ? "true": "false");
}

// recursive helper
char* __traverseHelper__(
    char* curr, int* shape, int* strides, int* backstrides, 
    int ndim, int depth
) {
    // we are at the last dimension
    if(depth == ndim - 1) {
        fprintf(stdout, "%.3f ", *(float*)curr);
        for(int i=0; i<shape[ndim-1] - 1; i++) {
            curr += strides[ndim - 1];
            fprintf(stdout, "%.3f ", *(float*)curr);
        }

        // backstep
        curr += backstrides[ndim - 1];
        printf("\n");
        return curr;
    }

    curr = __traverseHelper__(curr, shape, strides, backstrides, ndim, depth + 1);
    for(int i=0; i<shape[depth] - 1; i++) {
        curr += strides[depth];
        curr = __traverseHelper__(curr, shape, strides, backstrides, ndim, depth + 1);
    }
    
    curr += backstrides[depth];

    if(depth != 0)
        printf("\n");

    return curr;
}

/*
traverse array according to backstrides and execute a printf on each element
*/
void smShow(Array* arr) {
    char* curr = (char*)arr->data;

    curr = __traverseHelper__(
        curr, arr->shape, arr->strides, arr->backstrides,
        arr->ndim, 0
    );
}



// ------------------- Operations with Arrays -------------------

/*
- ndim should be equal
- each element in the shape should be equal
*/
int smCheckShapesEqual(Array* a, Array* b) {
    int equal = 1;
    
    if (a->ndim != b->ndim) equal = 0;

    else {
        for(int i=0; i<a->ndim; i++)
            if(a->shape[i] != b->shape[i]) {
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
int* __broadcastFinalShape__(Array* a, Array* b) {
    if(smCheckShapesEqual(a, b)) return a->shape;

    int res_ndim = (a->ndim > b->ndim) ? a->ndim: b->ndim;

    // left's 1s to prepend
    int l1add = res_ndim - a->ndim;
    int r1add = res_ndim - b->ndim;

    int lf_shape[res_ndim], rf_shape[res_ndim];
    int* res_shape = (int*)malloc(res_ndim * sizeof(int));

    // a
    int inda = 0;
    for(int i = 0; i<res_ndim; i++) {
        if(i < l1add) lf_shape[i] = 1;
        else {
            lf_shape[i] = a->shape[inda];
            inda++;
        }
    }

    // b
    int indb = 0;
    for(int i = 0; i<res_ndim; i++) {
        if(i < r1add) rf_shape[i] = 1;
        else {
            rf_shape[i] = b->shape[indb];
            indb++;
        }
    }

    for(int i=0; i<res_ndim; i++) {
        if(lf_shape[i] == 1 || rf_shape[i] == 1 || lf_shape[i] == rf_shape[i])
            res_shape[i] = (lf_shape[i] > rf_shape[i]) ? lf_shape[i]: rf_shape[i];
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
Array* __broadcastArray__(Array* arr, const int* shape, int ndim) {
    Array* res = smCreate(shape, ndim);

    bool do_copy = (res->totalsize == arr->totalsize) ? false : true;

    if(!do_copy) 
        smFromValues(res, arr->data);
    else {
        // resultant totalsize of broadcasting one Array (if possible) 
        // will always be a multiple of original Array's totalsize
        // example: (3, ) and (2, 2, 3)
        // when the index reaches the original array's boundary, wrap around to 0

        int boundary = arr->totalsize;
        for(int i=0; i<res->totalsize; i++) {
            res->data[i] = arr->data[i % boundary];
        }
    }

    return res;
}

/*
reshape an Array to new shape and new ndim
*/
Array* smReshapeNew(Array* arr, const int* shape, int ndim) {
    bool possible = __checkShapeCompatible__(arr, shape, ndim);
    if(!possible) {
        fprintf(stderr, "Cannot reshape due to invalid shape.\n");
        exit(1);
    }

    Array* res = smCreate(shape, ndim);
    smFromValues(res, arr->data);

    return res;
}

/*
inplace reshape operation

this will just change the shape and strides of the Array.
the underlying data in the memory is not touched.
*/
void smReshapeInplace(Array* arr, const int* shape, int ndim) {
    bool possible = __checkShapeCompatible__(arr, shape, ndim);
    if(!possible) {
        fprintf(stderr, "Cannot reshape due to invalid shape.\n");
        exit(1);
    }

    arr->ndim = ndim;
    for(int i=0; i<ndim; i++) {
        arr->shape[i] = shape[i];
    }

    __recalculateStrides__(arr);
    __recalculateBackstrides__(arr);
    __setArrayFlags__(arr);
}

/*
transpose an Array along given permutation of axes
assume axes is a valid permutation
*/
Array* smTransposeNew(Array* arr, const int* axes) {
    Array* res = smCreate(arr->shape, arr->ndim);
    if(arr->ndim == 1) {
        smFromValues(res, arr->data);
        return res;
    }

    // set data
    smFromValues(res, arr->data);

    int* _axes = (int*)malloc(res->ndim * sizeof(int));
    _checkNull(_axes);

    if(axes == NULL) {
        for(int i=0; i < res->ndim; i++) {
            _axes[i] = res->ndim - 1 - i;
        }
    }

    int* newshape = (int*)malloc(res->ndim * sizeof(int));
    int* newstrides = (int*)malloc(res->ndim * sizeof(int));
    _checkNull(newshape); _checkNull(newstrides);

    for(int i=0; i<res->ndim; i++) {
        newshape[i] = (axes != NULL) ? arr->shape[axes[i]] : arr->shape[_axes[i]];
        newstrides[i] = (axes != NULL) ? arr->strides[axes[i]] : arr->strides[_axes[i]];
    }
    for(int i=0; i<res->ndim; i++) {
        res->shape[i] = newshape[i];
        res->strides[i] = newstrides[i];
    }
    free(newshape); free(newstrides);
    __recalculateBackstrides__(res);
    __setArrayFlags__(res);

    free(_axes);
    return res;
}

/*
can be parallelized.
*/
Array* __PaddArrays__(Array* a, Array* b) {
    Array* res = smCreate(a->shape, a->ndim);

    #ifdef PARALLEL
    #pragma omp parallel for
    #endif
    for(int i=0; i<a->totalsize; i++) {
        res->data[i] = a->data[i] + b->data[i];
    }

    return res;
}


Array* __PmulArrays__(Array* a, Array* b) {
    Array* res = smCreate(a->shape, a->ndim);

    #ifdef PARALLEL
    #pragma omp parallel for
    #endif
    for(int i=0; i<a->totalsize; i++) {
        res->data[i] = a->data[i] * b->data[i];
    }

    return res;
}

/*
add the elements of two Arrays elementwise
if the shapes are not equal but broadcastable,
then broadcasting will take place.
*/
Array* smAdd(Array* a, Array* b) {
    if (smCheckShapesEqual(a, b))
        return __PaddArrays__(a, b);

    int* res_shape = __broadcastFinalShape__(a, b);
    
    if(res_shape == NULL) {
        fprintf(stderr, "Cannot add Arrays of non-broadcastable shapes.\n");
        exit(1);
    }

    // broadcast shapes
    // free broadcasts
    int res_ndim = (a->ndim > b->ndim) ? a->ndim: b->ndim;

    Array* afinal = __broadcastArray__(a, res_shape, res_ndim);
    Array* bfinal = __broadcastArray__(b, res_shape, res_ndim);

    Array* res = __PaddArrays__(afinal, bfinal);

    smCleanup(afinal); smCleanup(bfinal);
    free(res_shape);

    return res;
}

/*
-1 * arr->data
*/
Array* __PnegArray__(Array* arr) {
    Array* res = smCreate(arr->shape, arr->ndim);
    
    #ifdef PARALLEL
    #pragma omp parallel for
    #endif
    for(int i=0; i<res->totalsize; i++) {
        res->data[i] = -1 * arr->data[i];
    }

    return res;
}

/*
this is basically a + (-1 * b) elementwise
*/
Array* __PsubArrays__(Array* a, Array* b) {
    Array* bneg = __PnegArray__(b);

    Array* res = smAdd(a, bneg);

    smCleanup(bneg);
    return res;
}

/*
multiply the elements of two Arrays elementwise
for now, assume shapes of both are same

TODO: implement broadcasting
*/
Array* smMul(Array* a, Array* b) {
    if (smCheckShapesEqual(a, b))
        return __PmulArrays__(a, b);

    int* res_shape = __broadcastFinalShape__(a, b);
    
    if(res_shape == NULL) {
        fprintf(stderr, "Cannot multiply Arrays of non-broadcastable shapes.\n");
        exit(1);
    }

    // broadcast shapes
    // free broadcasts
    int res_ndim = (a->ndim > b->ndim) ? a->ndim: b->ndim;

    Array* afinal = __broadcastArray__(a, res_shape, res_ndim);
    Array* bfinal = __broadcastArray__(b, res_shape, res_ndim);

    Array* res = __PmulArrays__(afinal, bfinal);

    smCleanup(afinal); smCleanup(bfinal);
    free(res_shape);

    return res;
}

void smApply(Array *arr, ArrayFunc func) {
    // Array* res = smCreate(arr->shape, arr->ndim);
    
    #ifdef PARALLEL
    #pragma omp parallel for
    #endif
    for(int i=0; i<arr->totalsize; i++) {
        arr->data[i] = func(arr->data[i]);
    }

    // return res;
}

float square(float x) {
    return x * x;
}


int main() {
    // new random values 
    srand(time(NULL));

    const int shape[] = {5000, 5000};

    // 10,000 x 10,000 array
    const int shape[] = {1e4, 1e4};
    Array *a = smRandom(shape, 2);

    double start = omp_get_wtime();
    Array* res = smAdd(a, b);
    double end = omp_get_wtime();

    printf("smAdd: %.3f\n", end - start);

    #pragma omp parallel for
    for (int i = 0; i < res->totalsize; i++) {
        assert(res->data[i] == a->data[i] + b->data[i]);
    } 

    // smShow(res); 

    Array* res_copy = smCreate(res->shape, res->ndim);
    smFromValues(res_copy, res->data);
    start = omp_get_wtime();
    smApply(res, square);
    end = omp_get_wtime();

    printf("smApply: %.3f\n", end - start);

    #pragma omp parallel for
    for (int i = 0; i < res->totalsize; i++) {
        assert(res->data[i] == square(res_copy->data[i]));
    }

    // smShow(res);


    printf("\n>> elapsed time: %.5f ms\n\n", cpu_time);
    return 0;
}