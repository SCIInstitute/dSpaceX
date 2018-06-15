#pragma once

#include <png.h>
#include <string>

class Image {
public:
	int width;
	int height;
	unsigned char* imageData;
};

class ImageLoader {
public:
	Image loadImage(const std::string filename);
private:
	bool loadPNG(std::string filename, int *width, int *height, png_byte **imageData);
};
