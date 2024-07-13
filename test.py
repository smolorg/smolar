from smolar import Smolar
import numpy as np

def matMulTest():
    sm = Smolar()
    a = sm.smRandom((2, 50, 1000))
    b = sm.smRandom((2, 1000, 30))

    arr = sm.smMatMul(a, b)

    a_np = sm.toNumpy(a)
    b_np = sm.toNumpy(b)
    arr_np = sm.toNumpy(arr)
    print(a_np)
    print(b_np)
    print(arr_np)

    assert np.allclose(np.matmul(a_np, b_np), arr_np)
    print("Passed matmul test")

    sm.smCleanup(arr)
    sm.smCleanup(a)
    sm.smCleanup(b)

def testExpandDims():
    sm = Smolar()
    a = sm.smRandom((2, 3))
    arr = sm.smExpandDims(a, 0)
    arr = sm.toNumpy(arr)
    print(arr)
    assert arr.shape == (1, 2, 3)
    print("Passed expanddims test")
    sm.smCleanup(a)

def testReshape():
    sm = Smolar()
    a = sm.smRandom((4, 2, 3))
    arr = sm.smReshapeNew(a, (8, 3), 2)
    arr = sm.toNumpy(arr)
    print(arr)
    assert arr.shape == (8, 3)
    print("Passed reshape test")

    sm.smCleanup(a)

if __name__ == "__main__":
    matMulTest()
    testExpandDims()
    testReshape()



