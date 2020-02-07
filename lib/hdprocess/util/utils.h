#pragma once

#include "flinalg/Linalg.h"
#include "precision/Precision.h"

namespace HDProcess {

FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x);

}