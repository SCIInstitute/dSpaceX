#ifndef NEIGHBORHOOD_H
#define NEIGHBORHOOD_H

#include "flinalg/DenseMatrix.h"
#include "flinalg/SparseMatrix.h"

template <typename TPrecision>
class Neighborhood {                                          
  public:
    virtual FortranLinalg::SparseMatrix<TPrecision> generateNeighborhood(
        FortranLinalg::Matrix<TPrecision> &data) = 0;
};

#endif
