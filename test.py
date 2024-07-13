from smolar import Smolar

if __name__ == "__main__":
    sm = Smolar();
    a = sm.smRandom((2, 3))
    b = sm.smRandom((3, 4))

    arr = sm.smMatMul(a, b)
    sm.smShow(arr)
    print(sm.toNumpy(arr))

    sm.smCleanup(arr)   
    sm.smCleanup(a)
    sm.smCleanup(b)

    # ____________________________________________________________

    a = sm.smRandom((2, 2))
    sm.smShow(a)
    print(sm.toNumpy(a))
    sm.smCleanup(a)

    # ____________________________________________________________

    a = sm.smArange(1, 11, 1)
    sm.smShow(a)
    sm.smReshapeInplace(a, (5, 2), 2)
    sm.smShow(a)
    sm.smCleanup(a)



