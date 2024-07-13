import ctypes
import numpy as np

# Load the shared library
smolar_lib = ctypes.CDLL('./libsmolar.so')

class Smolar:
    def __init__(self):
        self.smolar = smolar_lib

        ArrayFunc = ctypes.CFUNCTYPE(ctypes.c_float, ctypes.c_float)
        class ArrayIndices(ctypes.Structure):
            _fields_ = [("count", ctypes.c_int),
                        ("indices", ctypes.POINTER(ctypes.POINTER(ctypes.c_int)))]
        
        class LinearIndices(ctypes.Structure):
            _fields_ = [("count", ctypes.c_int),
                        ("indices", ctypes.POINTER(ctypes.c_int))]
        
        class Array(ctypes.Structure):
            _fields_ = [("data", ctypes.POINTER(ctypes.c_float)),
                        ("shape", ctypes.POINTER(ctypes.c_int)),
                        ("strides", ctypes.POINTER(ctypes.c_int)),
                        ("backstrides", ctypes.POINTER(ctypes.c_int)),
                        ("ndim", ctypes.c_int),
                        ("itemsize", ctypes.c_int),
                        ("totalsize", ctypes.c_int),
                        ("idxs", ctypes.POINTER(ArrayIndices)),
                        ("lidxs", ctypes.POINTER(LinearIndices)),
                        ("C_ORDER", ctypes.c_bool),
                        ("F_ORDER", ctypes.c_bool)]

        # Set the return and argument types for the C functions
        self.smolar.smCreate.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]
        self.smolar.smCreate.restype = ctypes.POINTER(Array)

        self.smolar.smCleanup.argtypes = [ctypes.POINTER(Array)]
        self.smolar.smCleanup.restype = None

        self.smolar.smRandom.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]
        self.smolar.smRandom.restype = ctypes.POINTER(Array)

        self.smolar.smArange.argtypes = [ctypes.c_float, ctypes.c_float, ctypes.c_float]
        self.smolar.smArange.restype = ctypes.POINTER(Array)

        self.smolar.smFromValues.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(ctypes.c_float)]
        self.smolar.smFromValues.restype = None

        self.smolar.smPrintInfo.argtypes = [ctypes.POINTER(Array)]
        self.smolar.smPrintInfo.restype = None

        self.smolar.smCheckShapesEqual.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(Array)]
        self.smolar.smCheckShapesEqual.restype = ctypes.c_bool

        self.smolar.smReshapeNew.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(ctypes.c_int), ctypes.c_int]
        self.smolar.smReshapeNew.restype = ctypes.POINTER(Array)

        self.smolar.smReshapeInplace.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(ctypes.c_int), ctypes.c_int]
        self.smolar.smReshapeInplace.restype = None

        self.smolar.smTransposeNew.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(ctypes.c_int)]
        self.smolar.smTransposeNew.restype = ctypes.POINTER(Array)

        self.smolar.smAdd.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(Array)]
        self.smolar.smAdd.restype = ctypes.POINTER(Array)

        self.smolar.smMul.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(Array)]
        self.smolar.smMul.restype = ctypes.POINTER(Array)

        self.smolar.smExpandDims.argtypes = [ctypes.POINTER(Array), ctypes.c_int]
        self.smolar.smExpandDims.restype = ctypes.POINTER(Array)

        self.smolar.smSqueeze.argtypes = [ctypes.POINTER(Array), ctypes.c_int]
        self.smolar.smSqueeze.restype = ctypes.POINTER(Array)

        self.smolar.smDot.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(Array)]
        self.smolar.smDot.restype = ctypes.POINTER(Array)

        self.smolar.smMatMul.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(Array)]
        self.smolar.smMatMul.restype = ctypes.POINTER(Array)

        self.smolar.smApplyInplace.argtypes = [ctypes.POINTER(Array), ArrayFunc]
        self.smolar.smApplyInplace.restype = None

    def smCreate(self, shape: tuple):
        shape = (ctypes.c_int * len(shape))(*shape)
        return self.smolar.smCreate(shape, len(shape))
    
    def smCleanup(self, arr):
        self.smolar.smCleanup(arr)

    def smRandom(self, shape: tuple):
        shape = (ctypes.c_int * len(shape))(*shape)
        return self.smolar.smRandom(shape, len(shape))

    def smArange(self, start: float, end: float, step: float):
        return self.smolar.smArange(start, end, step)

    def smFromValues(self, arr, values: list):
        values = (ctypes.c_float * len(values))(*values)
        self.smolar.smFromValues(arr, values)

    def smPrintInfo(self, arr):
        self.smolar.smPrintInfo(arr)

    def smCheckShapesEqual(self, a, b):
        return self.smolar.smCheckShapesEqual(a, b)
    
    def smReshapeNew(self, arr, shape: tuple, ndim: int):
        shape = (ctypes.c_int * len(shape))(*shape)
        return self.smolar.smReshapeNew(arr, shape, ndim)

    def smReshapeInplace(self, arr, shape: tuple, ndim: int):
        shape = (ctypes.c_int * len(shape))(*shape)
        self.smolar.smReshapeInplace(arr, shape, ndim)

    def smTransposeNew(self, arr, axes: list):
        axes = (ctypes.c_int * len(axes))(*axes)
        return self.smolar.smTransposeNew(arr, axes)

    def smAdd(self, a, b):
        return self.smolar.smAdd(a, b)

    def smMul(self, a, b):
        return self.smolar.smMul(a, b)

    def smExpandDims(self, arr, axis: int):
        return self.smolar.smExpandDims(arr, axis)

    def smSqueeze(self, arr, axis: int):
        return self.smolar.smSqueeze(arr, axis)

    def smDot(self, a, b):
        return self.smolar.smDot(a, b)

    def smMatMul(self, a, b):
        return self.smolar.smMatMul(a, b)

    def smApplyInplace(self, arr, func):
        self.smolar.smApplyInplace(arr, func)

    def smShow(self, arr):
        self.smolar.smShow(arr)

    def smApply(self, arr, func):
        self.smolar.smApply(arr, func)

    def toNumpy(self, arr):
        data = tuple(arr.contents.data[i] for i in range(arr.contents.totalsize))
        shape = tuple(arr.contents.shape[i] for i in range(arr.contents.ndim))

        nparray = np.array(data, dtype=np.float32).reshape(shape)
        return nparray
    
    def smShape(self, arr):
        return tuple(arr.contents.shape[i] for i in range(arr.contents.ndim))
    

