#pragma once

#include "Precision.h"

namespace dspacex {

/// fieldvalue and the index of its sample in the full set of samples for a dataset
/// (and the index of the sample in a particular model, so it's a trio)
struct ValueIndexPair
{
  unsigned idx;       // global index of sample within the full dataset
  Precision val{0.0};
  int local_idx{-1};  // index within a specific crystal (for looking up z_coords of that crystal's model)

  static bool compare(const ValueIndexPair &p, const ValueIndexPair &q) { return p.val < q.val; }
};

// https://stackoverflow.com/a/46128321/207044 (for the set used to store crystals, until we're up to c++20)
struct ValueIndexPairCmp {
  bool operator()(const ValueIndexPair& lhs, const ValueIndexPair& rhs) const {
    return ValueIndexPair::compare(lhs, rhs);
  }
};

} // dspacex
