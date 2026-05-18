# libvector
This project contains a vector container as well as a string container built on top of it.
This implementation aims to be optimized and suitable for any case where a dynamically allocated array must be used in C. The string container is nothing more than a vector of char holding a nul byte termination invariant accross every method.