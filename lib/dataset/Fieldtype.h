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

  Fieldtype(const char type) : kind(type) {}

  operator char() const { assert(valid()); return kind; } // enables comparison using ==
  bool valid() const { return kind == DesignParameter || kind == QoI || kind == Unknown; }
  std::string asString() { // For exporting ms complex
    switch(kind) {
      case DesignParameter:
        return "Design Parameter";
      case QoI:
        return "QoI";
      case Unknown:
        return "Unknown Fieldtype";
      default:
        return "ERROR: Invalid Fieldtype";
    }
  }

  char kind;
  const static char DesignParameter = 5;
  const static char QoI = 10;
  const static char Unknown = -1;
};

} // dspacex
