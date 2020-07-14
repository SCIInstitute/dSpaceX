#include "Models.h"

namespace dspacex {

Model::Type Model::strToType(const std::string& type) {
  if (type == "pca")          return Model::PCA;
  if (type == "shapeodds")    return Model::ShapeOdds;
  if (type == "infshapeodds") return Model::InfShapeOdds;
  if (type == "sharedgp")     return Model::SharedGP;
  return None;
}

std::string Model::typeToStr(const Model::Type& type) {
  if (type == Model::PCA)          return "PCA";
  if (type == Model::ShapeOdds)    return "ShapeOdds";
  if (type == Model::InfShapeOdds) return "InfShapeOdds";
  if (type == Model::SharedGP)     return "SharedGP";
  return "<Unknown Model Type>";
}

std::ostream& operator<<(std::ostream &os, const Model::Type& type) {
  return os << Model::typeToStr(type);
}

} // dspacex
