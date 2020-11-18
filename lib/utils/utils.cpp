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

} // dspacex
