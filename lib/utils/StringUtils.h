#pragma once

#include <string>

std::string maybePadIndex(unsigned index, bool pad = false, unsigned num_indices = 0);

// returns a string containing index padded with zeros up to width characters
std::string paddedIndexString(unsigned index, unsigned width);

// returns the minimum string with necessary to represent num
unsigned paddedStringWidth(unsigned num);
