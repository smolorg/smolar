from smolar import Smolar

if __name__ == "__main__":
    sm = Smolar();
    a = sm.smRandom((10, 100))
    b = sm.smRandom((100, 5))

    arr = sm.smMatMul(a, b)

    sm.smShow(arr)      

    sm.smCleanup(arr)   
    sm.smCleanup(a)
    sm.smCleanup(b)

