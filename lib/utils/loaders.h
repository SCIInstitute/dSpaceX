#pragma once

#include "flinalg/Linalg.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/DenseVector.h"
#include "dataset/Precision.h"

#include <string>
#include <vector>

namespace HDProcess {

FortranLinalg::DenseMatrix<Precision> loadCSVMatrix(std::string filename);

FortranLinalg::DenseVector<Precision> loadCSVColumn(std::string filename);

FortranLinalg::DenseVector<Precision> loadCSVColumn(std::string filename, std::string columnName);

std::vector<std::string> loadCSVColumnNames(std::string filename);

}
