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

} // dspacex
