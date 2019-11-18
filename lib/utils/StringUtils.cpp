#include "StringUtils.h"

#include <sstream>
//#include <memory>
#include <iomanip>

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
