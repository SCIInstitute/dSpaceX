#pragma once

#include <string>

// HDVizLayout
//
// example:
// HDVizLayout type("iso");
// if (type == HDVizLayout::ISOMAP) ...
struct HDVizLayout {
  HDVizLayout() : kind(Unknown) {}
  HDVizLayout(const std::string strtype)
  {
    if (strtype == "isomap" || strtype == "iso")
      kind = ISOMAP;
    else if (strtype == "pca")
      kind = PCA;
    else if (strtype == "pca2")
      kind = PCA2;
    else
      kind = Unknown;
  }

  HDVizLayout(const char type) : kind(type) {}

  operator char() const { assert(valid()); return kind; } // enables comparison using ==
  bool valid() const { return kind == ISOMAP || kind == PCA || kind == PCA2; }
  std::string asString() { // For exporting
    switch(kind) {
      case ISOMAP:
        return "ISOMAP";
      case PCA:
        return "PCA";
      case PCA2:
        return "PCA2";
      case Unknown:
        return "Unknown HDVizLayout";
      default:
        return "ERROR: Invalid HDVizLayout";
    }
  }

  char kind;
  const static char ISOMAP = 5;
  const static char PCA = 10;
  const static char PCA2 = 15;
  const static char Unknown = -1;
};
