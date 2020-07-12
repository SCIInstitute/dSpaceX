#include "Models.h"

namespace dspacex {

std::ostream& operator<<(std::ostream &os, const Model::Type& type) {
  if (type == Model::PCA)          return os << "PCA";
  if (type == Model::ShapeOdds)    return os << "ShapeOdds";
  if (type == Model::InfShapeOdds) return os << "InfShapeOdds";
  if (type == Model::SharedGP)     return os << "SharedGP";
  return os << "<Unknown Model Type>";
}

} // dspacex
