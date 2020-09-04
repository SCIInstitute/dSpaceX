#include "StringUtils.h"

#include <sstream>
#include <iomanip>

std::string maybePadIndex(unsigned index, bool pad, unsigned num_indices)
{
  if (pad) 
    return paddedIndexString(index, paddedStringWidth(num_indices));
  else
    return std::to_string(index);
}

// returns a string containing index padded with zeros up to width characters
std::string paddedIndexString(unsigned index, unsigned width)
{
  std::stringstream paddedIndex;
  paddedIndex << std::setfill('0') << std::setw(width) << index;
  return paddedIndex.str();
}

// returns the minimum string with necessary to represent num
unsigned paddedStringWidth(unsigned num)
{
  std::stringstream digitCounter;
  digitCounter << num;
  return digitCounter.str().size();
}
