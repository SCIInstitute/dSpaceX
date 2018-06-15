#pragma once

#include <png.h>
#include <string>

class ImageLoader {
public:
	bool loadPNG(std::string filename, int *width, int *height, png_byte **imageData);
private:
};
