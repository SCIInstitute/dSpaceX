#pragma once

#include <string>

namespace dspacex {

// Fieldtype: a quantity of interest or a design parameter
//
// example:
// Fieldtype type("qoi");
// if (type == Fieldtype::DesignParameter) ...
struct Fieldtype {
  Fieldtype(const std::string strtype)
  {
    if (strtype == "parameter")
      kind = DesignParameter;
    else if (strtype == "qoi")
      kind = QoI;
    else
      kind = Unknown;
  }

  Fieldtype(const int type) : kind(type) {}

  operator int() const { return kind; } // enables comparison using ==
  bool valid() { return kind == DesignParameter || kind == QoI; }
  std::string asString() { // For exporting ms complex
    switch(kind) {
        case DesignParameter:
          return "Design Parameter";
        case QoI:
          return "QoI";
    }
  }

  int kind;
  const static int DesignParameter = 0;
  const static int QoI = 1;
  const static int Unknown = -1;
};

} // dspacex
