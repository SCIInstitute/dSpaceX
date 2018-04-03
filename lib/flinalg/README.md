Linear algebra C++ wrapper for lapack blas and a randomized fast SVD implementation. 
Includes Matrix and Vector classes and i/o into a binary matrix format and wrappers to read and write from MATLAB and R. 

### Usage ###

* Requires lapack gfortran and blas libraires
* Uses CMake to build
     * Includes an R package and MATLAB build 
     * A few command line tools for testing and matrix file manipulations and conversions
* For use as library simple include files in lib dir 


### File format ###
Read and write binary matrix files with / from human readble header file.

Matrix File format:

The matrix and vector file is a simple binary file format with an additional
header file. The header file for the matrix looks as follows:

DenseMatrix
Size: 8 x 1030
ElementSize: 8
RowMajor: 0
DataFile: X.data

First line is the type of matrix
Second line is the matrix size
Third line is the element size, this application requires doubles (8 bytes) unless compiled with the precision set to float.
Fourth line wether the binary data is stored in row or column major format (0 = ColumnMajor otherwise RowMajor, default is ColumMajor for ease of
interaction with lapack)
Fifth line is a reference to the binary file.

The file format for vectors is in the same fashion:

DenseVector
Size: 1030
ElementSize: 8
DataFile: y.data