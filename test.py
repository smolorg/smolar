from smolar import Smolar

if __name__ == "__main__":
    sm = Smolar();
    a = sm.smRandom((2, 3))
    b = sm.smRandom((3, 4))

    arr = sm.smMatMul(a, b)

    # sm.smShow(arr)      

    sm.smCleanup(arr)   
    sm.smCleanup(a)
    sm.smCleanup(b)

    a = sm.smRandom((2, 2))
    sm.smShow(a)
    print(sm.toNumpy(a))

