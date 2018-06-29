#include "Image.h"

Image::Image(int width, int height, unsigned char *data, 
  std::vector<char> rawData, const std::string &format) : m_width(width), 
  m_height(height), m_data(data), m_rawData(rawData), m_format(format) {
  // intentionally left empty
}

unsigned char* Image::getData() {
  return m_data;
}

std::vector<char>& Image::getRawData() {
  return m_rawData;
}

int Image::getWidth() {
  return m_width;
}

int Image::getHeight() {
  return m_height;
}

std::string Image::getFormat() {
  return m_format;
}