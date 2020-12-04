#include "utils.h"

namespace dspacex {

// reinventing Windows... 
std::string uniqueFilename(const std::string &baseName, const std::string &ext) {
  std::string filename(baseName), basename(filename);

  std::ifstream f(filename + ".csv");
  int suffix(0);
  while (!f.good()) {
    // if output file already exists, increment suffix till it's unique
    f.open(basename + "(" + std::to_string(suffix) + ").csv");
    filename = basename + "(" + std::to_string(suffix) + ")";
  }

  return filename + ".csv";
}

std::string filepath(std::string basepath, std::string filename) {
  size_t found = basepath.find_last_of("/\\");
  std::string path = basepath.substr(0,found);
  if(path.empty()) {
    path = "./";
  }
  if (path[path.size() - 1] != '/') {
    path = path + "/";
  }
  return path + filename;
}

Precision computeSigma(const std::vector<Precision>& vals) {
  // compute pairwise distances
  using DistMat = Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic>;
  DistMat distances = DistMat::Zero(vals.size(), vals.size());
  for (int i = 0; i < vals.size(); i++) {
    for (int j = i + 1; j < vals.size(); j++) {
      distances(i, j) = std::abs(vals[i] - vals[j]);
    }
  }

  // fill other half of matrix
  DistMat the_other_half{distances.transpose()};
  distances += the_other_half;

  // set diagonals to very large value
  DistMat diag = DistMat::Identity(vals.size(), vals.size()) * std::numeric_limits<Precision>::max();
  distances += diag;

  // find min distance between each value and all the others
  EigenVectorX mins(vals.size());
  for (int i = 0; i < vals.size(); i++) {
    mins(i) = distances.row(i).minCoeff();
  }
  
  // return average of minvals
  return mins.sum() / vals.size();
}

FortranLinalg::DenseVector<Precision> normalize(const FortranLinalg::DenseVector<Precision>& values) {
  Eigen::Map<Eigen::Matrix<Precision, Eigen::Dynamic, 1>> _values(const_cast<FortranLinalg::DenseVector<Precision>&>(values).data(), values.N());
  FortranLinalg::DenseVector<Precision> ret(values.N()); 
  Eigen::Map<Eigen::Matrix<Precision, Eigen::Dynamic, 1>> _ret(ret.data(), ret.N());
  _ret = _values;

  auto showStats(true);
  if (showStats) {
    std::cout << "Raw field stats:\n";
    displayFieldStats(_ret);
  }

  // Scale normalize so that all values are in range [0,1]: for each member X: X = (X - min) / (max - min).
  auto minval(_ret.minCoeff());
  auto maxval(_ret.maxCoeff());
  _ret.array() -= minval;
  _ret.array() /= (maxval - minval);

  if (showStats) {
    std::cout << "Scale-normalized field stats:\n";
    displayFieldStats(_ret);
  }

  return ret;
}

} // dspacex
