#pragma once

#include "Image.h"
#include <string>

class ImageLoader {
public:
  enum class Format { PNG };
  Image loadImage(const std::string &filename, ImageLoader::Format format);
  Image loadPNG(const std::string &filename);
};
