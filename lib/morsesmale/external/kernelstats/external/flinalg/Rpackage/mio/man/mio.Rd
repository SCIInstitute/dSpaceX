\name{mio}
\alias{write.matrix}
\alias{read.matrix}
\title{Matrix IO}
\description{
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
Second line is the matriz size
Third line is the element size, this application requires doubles (8 bytes)
  unless compilled with the precision set to float.
Fourth line wether the binary data is stored in row or column major format (0 = ColumnMajor otherwise RowMajor, default is ColumMajor for ease of
interaction with lapack)
Fifth line is a reference to the binary file.

The file format for vectors is in the same fashionL:

DenseVector
Size: 1030
ElementSize: 8
DataFile: y.data




}
\usage{
write.matrix.double(x, filename)
read.matrix.double(filename)
write.matrix.int(x, filename)
read.matrix.int(filename)
write.vector.double(x, filename)
read.vector.double(filename)
write.vector.int(x, filename)
read.vector.int(filename)
}
\arguments{
  \item{x}{matrix to write}
  \item{filename}{file to read/write from (with extension for reading, with
out for writing)}
}
\author{
  Samuel Gerber
}
\examples{
library(mio)

X= matrix(1, nrow=2, ncol=4)

write.matrix.double(X, "test")
X = read.matrix.double("test.hdr")

}
\keyword{io}
