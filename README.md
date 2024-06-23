# smolar

A tiny multidimensional array implementation in C similar to numpy, but only one file.

I wanted to dive into the trenches of implementing multidimensional arrays since quite some time. Finally, taking inspiration from [numpy](https://numpy.org) I decided to give it a go. I wanted to implement everything from ground up, and hence C was the perfect choice for it.

### Working

Currently, `smolar` only supports N-d arrays of the `float` datatype. That is, every element in the N-d array will be a `float`. I did this because:

1. Integers can also be represented by floats (even though float takes more bytes in memory).
2. And for educational/learning purposes I just wanted to use float.
3. Support for more datatypes might be added in the future.

### File structure

There is only one file: `smolar.c`

Everything, and I mean EVERYTHING is in that file.

To compile and run this file using clang compiler:

```shell
$ clang smolar.c -o smolar
$ ./smolar
```


### Current progress

- [x] Define Array structure
- [x] Create/Init Array from shape
- [x] Traverse Array using strides
- [x] Ability to broadcast an array
- [x] Elementwise addition operation
- [ ] More Unary and Binary Array operations
- [ ] Support more `dtypes`
- [ ] Parallelism loops in Array operations
