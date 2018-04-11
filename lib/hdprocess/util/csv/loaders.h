#pragma once

#include "flinalg/Linalg.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/DenseVector.h"
#include "precision/Precision.h"

#include <string>


namespace HDProcess {

FortranLinalg::DenseMatrix<Precision> loadCSVMatrix(std::string filename);

FortranLinalg::DenseVector<Precision> loadCSVColumn(std::string filename);

FortranLinalg::DenseVector<Precision> loadCSVColumn(std::string filename, std::string columnName);
 
}