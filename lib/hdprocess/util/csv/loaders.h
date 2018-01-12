#pragma once

#include <string>

#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"

namespace HDProcess {

FortranLinalg::DenseMatrix<Precision> loadCSVMatrix(std::string filename);

FortranLinalg::DenseVector<Precision> loadCSVColumn(std::string filename, std::string columnName);
 
}