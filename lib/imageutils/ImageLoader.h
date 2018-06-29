#pragma once

#include "Image.h"
#include <string>
#include <vector>

class ImageLoader {
public:
  enum class Format { PNG, RAW_PNG };
  Image loadImage(const std::string &filename, ImageLoader::Format format);
  Image loadPNG(const std::string &filename);
private:
  std::vector<char> loadRawData(const std::string &filename);
};
