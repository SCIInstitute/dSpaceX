#pragma once

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
      kind = Invalid;
  }

  Fieldtype(const int type) : kind(type) {}

  operator int() const { return kind; } // enables comparison using ==
  bool valid() { return kind == DesignParameter || kind == QoI; }

  int kind;
  const static int DesignParameter = 0;
  const static int QoI = 1;
  const static int Invalid = -1;
};

