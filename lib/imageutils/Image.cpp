#include "Image.h"

Image::Image(int width, int height, unsigned char *data)
	: m_width(width), m_height(height), m_data(data) {
  // intentionally left empty
}

unsigned char* Image::getData() {
	return m_data;
}

int Image::getWidth() {
  return m_width;
}

int Image::getHeight() {
  return m_height;
}
