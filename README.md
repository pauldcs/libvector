# libvector
This project implements a vector container as well as a string container built on top of it.
It aims to be optimized and suitable for any case where a contiguous dynamically allocated storage must be used in C. The string container is nothing more than a vector of char holding a nul byte termination invariant accross every method.

## Build
### Debug
```
make g
```
### Release
```
make
```
### Run tests
Note: if you previously built the vector for release, run `make fclean` beforehand
to make sure to have everything compiled under debug to benefit from ASAN
```
make test
```

# Linking from your project
Compile (see above), grab `vector.h` from `incs` (or symlink to it),
and add this to your build system
```
-L<this directory> -lvector
```
