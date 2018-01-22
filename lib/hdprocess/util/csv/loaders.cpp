#include "loaders.h"
#include "csv.h"

#include <cassert>

namespace HDProcess {

FortranLinalg::DenseMatrix<Precision> loadCSVMatrix(std::string filename) {
  std::ifstream fstream(filename);
  std::string line;
  std::string token;

  std::vector<std::vector<double>> matrix;
  while (std::getline(fstream, line)) {
    std::istringstream ss(line);
    std::vector<double> row;
    while (std::getline(ss, token, ',')) {
      row.push_back(std::stod(token));
    }
    matrix.push_back(row);
  }

  FortranLinalg::DenseMatrix<Precision> m(matrix.size(),matrix[0].size());
  for (int i = 0; i < matrix.size(); i++) {
    for (int j = 0; j < matrix[0].size(); j++) {
      m(i,j) = matrix[i][j];
    }
  }

  return m;
}

FortranLinalg::DenseVector<Precision> loadCSVColumn(std::string filename) {
  std::ifstream fstream(filename);
  std::string line;
  std::string token;

  std::vector<double> vector;
  while (std::getline(fstream, line)) {
    std::istringstream ss(line);
    std::vector<double> row;
    while (std::getline(ss, token, ',')) {
      row.push_back(std::stod(token));
    }
    vector.push_back(row[0]);
  }

  FortranLinalg::DenseVector<Precision> v(vector.size());
  for (int i = 0; i < vector.size(); i++) {
    v(i) = vector[i];
  }

  return v;
}

FortranLinalg::DenseVector<Precision> loadCSVColumn(std::string filename, std::string columnName) {
  io::CSVReader<1> in(filename.c_str());
  in.read_header(io::ignore_extra_column, columnName);
  double value;
  std::vector<double> vec;
  while (in.read_row(value)) {
    vec.push_back(value);
  }

  FortranLinalg::DenseVector<Precision> v(vec.size());
  for (int i = 0; i < vec.size(); i++) {
    v(i) = vec[i];
  }

  return v;
}

} // namespace HDProcess