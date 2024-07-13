import ctypes
import numpy as np

# Load the shared library
smolar = ctypes.CDLL('./libsmolar.so')

# Define the ArrayFunc type
ArrayFunc = ctypes.CFUNCTYPE(ctypes.c_float, ctypes.c_float)

# Define the ArrayIndices struct in Python
class ArrayIndices(ctypes.Structure):
    _fields_ = [("count", ctypes.c_int),
                ("indices", ctypes.POINTER(ctypes.POINTER(ctypes.c_int)))]
    
# Define the LinearIndices struct in Python
class LinearIndices(ctypes.Structure):
    _fields_ = [("count", ctypes.c_int),
                ("indices", ctypes.POINTER(ctypes.c_int))]

# Define the Array struct in Python
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
smolar.smRandom.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]
smolar.smRandom.restype = ctypes.POINTER(Array)

smolar.smMatMul.argtypes = [ctypes.POINTER(Array), ctypes.POINTER(Array)]
smolar.smMatMul.restype = ctypes.POINTER(Array)

smolar.smShow.argtypes = [ctypes.POINTER(Array)]
smolar.smShow.restype = None

smolar.smCleanup.argtypes = [ctypes.POINTER(Array)]
smolar.smCleanup.restype = None

if __name__ == "__main__":
    # Create a 2x3 random array
    shape1 = (ctypes.c_int * 2)(2, 3)
    a = smolar.smRandom(shape1, 2)

    shape2 = (ctypes.c_int * 2)(3, 2)
    b = smolar.smRandom(shape2, 2)

    # Perform matrix multiplication
    arr = smolar.smMatMul(a, b)
    
    # Show the array
    smolar.smShow(arr)

    # cleanup
    smolar.smCleanup(arr)
