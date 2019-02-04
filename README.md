# clibarchive
An archive processor and compressor for C++ with zlib

It stores and compresses files in a file that is easy for programs to access.
There is currently only read/write functions to a file path, and no way of
knowing if a file exists or not.

Todo:
- add file existance checking
- add directory listing

To build:

`g++ archive.cpp compress.cpp -lz`
