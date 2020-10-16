#pragma once

#include "flinalg/Linalg.h"
#include "dspacex/Precision.h"

#include <Eigen/Dense>

namespace HDProcess {

FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x);

}

namespace dspacex {

/*
 * Converts mat to a FortranLinalg::DenseMatrix, taking data from mat (thus non-const and empty of data at the end).
 */
template<typename T>
static FortranLinalg::DenseMatrix<T> toDenseMatrix(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& mat)
{
  FortranLinalg::DenseMatrix<T> ret(mat.rows(), mat.cols(), mat.data());

  // use placement new to ensure data doesn't get reclaimed when mat goes out of scope by caller
  new (&mat) Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>(nullptr, -1, -1);

  return std::move(ret);
}

} // dspacex
