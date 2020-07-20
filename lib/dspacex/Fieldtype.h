#pragma once

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

/// fieldvalue and the index of its sample in the full set of samples for a dataset
struct ValueIndexPair
{
  float val;
  unsigned idx;

  static bool compare(const ValueIndexPair &p, const ValueIndexPair &q) { return p.val < q.val; }
};

} // dspacex
