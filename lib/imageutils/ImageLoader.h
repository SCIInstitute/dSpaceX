#pragma once

#include <png.h>
#include <string>

#include "Image.h"

class ImageLoader {
public:
	Image loadImage(const std::string filename);
private:
	bool loadPNG(std::string filename, int *width, int *height, png_byte **imageData);
};
